#include "UdpSocket.h"
#include "EvLoop.h"
#include <string.h>
#include <errno.h>

namespace Evel {

UdpSocket::UdpSocket( unsigned inp_buf_size, bool need_wait ) : Event( socket( AF_INET6, SOCK_NONBLOCK | SOCK_DGRAM, IPPROTO_UDP ), need_wait ), inp_buf_size( inp_buf_size ), buffer( 0 ) {
}

UdpSocket::~UdpSocket() {
    if ( buffer )
        free( buffer );
    for( SendItem &send : waiting_sends )
        msg_free( send.data );
}

void UdpSocket::bind( int port ) {
    // no "Address already in use" error message
    int yes = 1;
    while ( true ) {
        if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( yes ) ) < 0 ) {
            if ( errno == EINTR )
                continue;
            ev_loop->err( "setsockopt( SO_REUSEADDR ) failed: {}", strerror( errno ) );
        }
        break;
    }
    // #ifdef SO_REUSEPORT
    while ( true ) {
        if ( setsockopt( fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof( yes ) ) < 0 ) {
            if ( errno == EINTR )
                continue;
            ev_loop->err( "setsockopt( SO_REUSEPORT ) failed: {}", strerror( errno ) );
        }
        break;
    }

    // bind
    InetAddress ia( "::", port );
    if ( ::bind( fd, (const struct sockaddr *)&ia.sa, sizeof ia.sa ) < 0 )
        on_bind_error( strerror( errno ) );
}

void UdpSocket::send( const InetAddress &dst, const char *data ) {
    send( dst, data, strlen( data ) );
}

void UdpSocket::send( const InetAddress &dst, const char *data, size_t size ) {
    send( dst, &data, size, false );
}

void UdpSocket::send( const InetAddress &dst, char **data, size_t size, bool allow_transfer_ownership ) {
    send( dst, (const char **)data, size, allow_transfer_ownership );
}

void UdpSocket::send( const InetAddress &dst, const char **data, size_t size, bool allow_transfer_ownership ) {
    send_raw( dst, data, size, allow_transfer_ownership );
}

void UdpSocket::on_bind_error( const char *msg ) {
    ev_loop->err( "Bind error: ", msg );
    del();
}

void UdpSocket::on_inp() {
    while ( true ) {
        // try to read some data
        InetAddress src;
        if ( ! buffer )
            buffer = (char *)allocate( inp_buf_size );

        //
        //        struct iovec iov[ 1 ];
        //        iov[ 0 ].iov_base = (void *)buffer;
        //        iov[ 0 ].iov_len  = inp_buf_size;

        //        struct msghdr message;
        //        message.msg_name       = (sockaddr *)&src.sa; // res->ai_addr;
        //        message.msg_namelen    = sizeof src.sa; // res->ai_addrlen;
        //        message.msg_iov        = iov;
        //        message.msg_iovlen     = 1;
        //        message.msg_control    = 0;
        //        message.msg_controllen = 0;
        //        message.msg_flags      = 0;

        //        ssize_t inp_len = recvmsg( fd, &message, MSG_DONTWAIT ); // MSG_WAITALL
        //        P( inp_len, message.msg_flags, message.msg_flags, src );

        socklen_t src_sa_len = sizeof src.sa;
        ssize_t inp_len = recvfrom( fd, buffer, inp_buf_size, MSG_DONTWAIT, (struct sockaddr *)&src.sa, &src_sa_len ); // MSG_WAITALL
        if ( inp_len <= 0 ) {
            if ( errno == EINTR )
                continue;
            if ( errno == EAGAIN or errno == EWOULDBLOCK )
                return;
            // DISP_ERROR( "recvfrom {} (errno={},inp_len={})", strerror( errno ), errno, inp_len );
            return sys_error();
        }

        // parse
        parse( src, &buffer, inp_len );

        // early exit if next input is going to be ignored anyway
        if ( next_del || want_close_fd )
            return;
    }
}

void UdpSocket::on_out() {
    while ( ! waiting_sends.empty() ) {
        const SendItem &p = waiting_sends.front();

        while ( true ) {
            ssize_t real = sendto( fd, p.data, p.size, 0, (const sockaddr *)&p.dst.sa, sizeof p.dst.sa ); // MSG_NOSIGNAL
            if ( real <= 0 ) {
                if ( errno == EINTR )
                    continue;
                if ( errno == EAGAIN || errno == EWOULDBLOCK )
                    return;
                return sys_error();
            }
            if ( real != p.size )
                ev_loop->err( "real != size in UdpSocket::on_out()" );
            break;
        }

        msg_free( p.data );
        waiting_sends.pop_front();
    }
}

void *UdpSocket::allocate( size_t inp_buf_size ) {
    return malloc( inp_buf_size );
}

void UdpSocket::send_raw( const InetAddress &dst, const char **data, size_t size, bool allow_transfer_ownership ) {
    if ( size == 0 )
        return;

    if ( ! waiting_sends.empty() )
        return add_send_item( dst, data, size, allow_transfer_ownership );

    while ( true ) {
        // struct iovec iov[ 1 ];
        // iov[ 0 ].iov_base = (void *)*data;
        // iov[ 0 ].iov_len  = size;
        //
        // struct msghdr message;
        // message.msg_name       = (sockaddr *)&dst.sa; // res->ai_addr;
        // message.msg_namelen    = sizeof dst.sa; // res->ai_addrlen;
        // message.msg_iov        = iov;
        // message.msg_iovlen     = 1;
        // message.msg_control    = 0;
        // message.msg_controllen = 0;
        // message.msg_flags      = 0;

        ssize_t real = sendto( fd, *data, size, 0, (const sockaddr *)&dst.sa, sizeof dst.sa ); // MSG_NOSIGNAL
        // ssize_t real = sendmsg( fd, &message, 0 ); // MSG_NOSIGNAL
        if ( real <= 0 ) {
            if ( errno == EINTR )
                continue;
            if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
                if ( allow_transfer_ownership ) {
                    waiting_sends.emplace_back( SendItem{ dst, *data, size } );
                    data = 0;
                } else {
                    char *n_data = (char *)malloc( size );
                    memcpy( n_data, *data, size );
                    waiting_sends.emplace_back( SendItem{ dst, n_data, size } );
                }
                return;
            }
            return on_hup();
        }
        if ( real != size )
            ev_loop->err( "real != size in UdpSocket::send_raw" );
        break;
    }
}

void UdpSocket::add_send_item( const InetAddress &dst, const char **data, size_t size, bool allow_transfer_ownership ) {
    if ( allow_transfer_ownership ) {
        waiting_sends.emplace_back( SendItem{ dst, *data, size } );
        data = 0;
    } else {
        char *n_data = (char *)msg_alloc( size );
        memcpy( n_data, *data, size );
        waiting_sends.emplace_back( SendItem{ dst, n_data, size } );
    }
}

void *UdpSocket::msg_alloc( size_t size ) {
    return malloc( size );
}

void UdpSocket::msg_free( const void *ptr ) {
    free( (void *)ptr );
}

bool UdpSocket::out_are_sent() const {
    return waiting_sends.empty();
}

} // namespace Evel
