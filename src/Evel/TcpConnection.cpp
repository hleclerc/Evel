#include "TcpConnection.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include "EvLoop.h"

namespace Evel {

TcpConnection::TcpConnection( const InetAddress &addr ) : waiting_for_connection( true ), inp_buffer( 0 ) {
    // create an ipv6 socket
    while ( true ) {
        fd = socket( AF_INET6, SOCK_NONBLOCK | SOCK_STREAM, 0 );
        if ( fd < 0 ) {
            if ( errno == EINTR )
                continue;
            ev_loop->err( "socket error: ", strerror( errno ) );
            return;
        }
        break;
    }

    // connect
    while ( true ) {
        if ( connect( fd, (const struct sockaddr *)&addr.sa, sizeof addr.sa ) < 0 ) {
            if ( errno == EINPROGRESS )
                break;
            if ( errno == EINTR )
                continue;
            ev_loop->err( "connect error: ", strerror( errno ) );
            return;
        }
        break;
    }
}

TcpConnection::TcpConnection( int accepting_fd ) : waiting_for_connection( false ), inp_buffer( 0 ) {
    fd = accepting_fd;
}

TcpConnection::TcpConnection( VtableOnly vo ) : Event( vo ) {
}

TcpConnection::~TcpConnection() {
    for( SendItem &si : waiting_sends )
        msg_free( si.allo );
}

void TcpConnection::send( const char *data ) {
    send( data, strlen( data ) );
}

void TcpConnection::send( const char *data, size_t size ) {
    send( &data, size, size, false );
}

void TcpConnection::send( const char **data, size_t size, size_t rese, bool allow_transfer_ownership ) {
    if ( size == 0 )
        return;

    if ( waiting_for_connection || ! waiting_sends.empty() )
        return add_send_item( data, size, rese, allow_transfer_ownership );

    // only one message
    iovec iov;
    iov.iov_base = (void *)*data;
    iov.iov_len  = size;

    // sendmsg
    msghdr msg; bzero( &msg, sizeof msg );
    msg.msg_iovlen = 1;
    msg.msg_iov    = &iov;

    while ( true ) {
        ssize_t real = sendmsg( fd, &msg, MSG_NOSIGNAL );
        if ( real <= 0 ) { // error ?
            if ( errno == EINTR )
                continue;
            if ( real < 0 and ( errno == EAGAIN or errno == EWOULDBLOCK ) )
                return;
            return on_hup();
        }
        break;
    }
}

void TcpConnection::add_send_item( const char **data, size_t size, size_t rese, bool allow_transfer_ownership ) {
    if ( size ) {
        if ( allow_transfer_ownership ) {
            waiting_sends.emplace_back( SendItem{ *data, *data, size, rese } );
            data = 0;
        } else {
            char *n_data = (char *)msg_alloc( size );
            memcpy( n_data, *data, size );
            waiting_sends.emplace_back( SendItem{ n_data, n_data, size, rese } );
        }
    }
}

void *TcpConnection::msg_alloc( size_t size ) {
    return malloc( size );
}

void TcpConnection::msg_free( const void *ptr ) {
    free( (void *)ptr );
}

void TcpConnection::on_inp() {
    while ( want_close_fd == false && next_del == 0 ) {
        // try to find the message length
        unsigned buff_size, max_buff_size = 1u << 20;
        while ( true ) {
            if ( ioctl( fd, FIONREAD, &buff_size ) == -1 ) {
                if ( errno == EINTR )
                    continue;
                ev_loop->log( "ioctl FIONREAD: {}", strerror( errno ) );
                buff_size = 8192;
            } else if ( buff_size > max_buff_size )
                buff_size = max_buff_size;
            break;
        }

        // if no available inp_buffer or if size is not high enough, create a new one
        if ( ! inp_buffer ) {
            inp_buffer = (char *)msg_alloc( buff_size );
            inp_buffer_size = buff_size;
        } else if ( inp_buffer_size < buff_size ) {
            msg_free( inp_buffer );
            inp_buffer = (char *)msg_alloc( buff_size );
            inp_buffer_size = buff_size;
        }

        // try to read some data
        ssize_t used = recv( fd, inp_buffer, inp_buffer_size, MSG_DONTWAIT );
        if ( used <= 0 ) {
            if ( errno == EINTR )
                continue;
            if ( used and ( errno == EAGAIN or errno == EWOULDBLOCK ) )
                return;
            return on_hup();
        }

        // parse
        parse( &inp_buffer, used, inp_buffer_size );
    }
}

void TcpConnection::on_out() {
    // all the messages
    size_t nmsg = waiting_sends.size(), tot = 0;
    if ( viovec.size() < nmsg )
        viovec.resize( nmsg );
    iovec *iov_ptr = viovec.data();
    for( SendItem &si : waiting_sends ) {
        iov_ptr->iov_base = (void *)si.data;
        iov_ptr->iov_len  = si.size;
        tot += si.size;
        ++iov_ptr;
    }
    if ( tot == 0 )
        return;

    // preparation for message structure
    msghdr msg; bzero( &msg, sizeof msg );
    iov_ptr = viovec.data();

    // sendmsg
    while ( true ) {
        msg.msg_iovlen = nmsg;
        msg.msg_iov    = iov_ptr;

        ssize_t real = sendmsg( fd, &msg, MSG_NOSIGNAL );
        if ( real <= 0 ) { // error ?
            if ( errno == EINTR )
                continue;
            if ( real < 0 and ( errno == EAGAIN or errno == EWOULDBLOCK ) )
                return;
            return on_hup();
        }
        if ( real == tot ) {
            for( SendItem &si : waiting_sends )
                msg_free( si.allo );
            waiting_sends.clear();
            break;
        }

        // partly sent
        while ( real >= iov_ptr->iov_len ) {
            SendItem &si = waiting_sends.front();
            msg_free( si.allo );
            waiting_sends.pop_front();
            real -= iov_ptr->iov_len;
            ++iov_ptr;
            --nmsg;
        }
        SendItem &si = waiting_sends.front();
        iov_ptr->iov_len  -= real;
        iov_ptr->iov_base  = (char *)iov_ptr->iov_base + real;
        si.data += real;
        si.size -= real;
    }
}

bool TcpConnection::out_are_sent() const {
    return waiting_sends.empty();
}


} // namespace Evel
