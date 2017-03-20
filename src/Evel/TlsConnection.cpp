#include "TlsConnection.h"
#include <openssl/err.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include "EvLoop.h"
#include "Print.h"

namespace Evel {

TlsConnection::TlsConnection( SSL_CTX *ssl_ctx, const InetAddress &addr, const char *pref_ciph ) : TcpConnection( addr ), comm_mode( CommMode::Client ) {
    _init( ssl_ctx, pref_ciph );
}

TlsConnection::TlsConnection( SSL_CTX *ssl_ctx, int accepting_fd, const char *pref_ciph ) : TcpConnection( accepting_fd ), comm_mode( CommMode::Server ) {
    _init( ssl_ctx, pref_ciph );
}

void TlsConnection::_init( SSL_CTX *ssl_ctx, const char *pref_ciph ) {
    to_redo              = ToRedo::Nothing;
    want_a_shutdown      = false;
    started_a_shutdown   = false;
    has_waiting_inp_data = false;

    ssl = SSL_new( ssl_ctx );
    SSL_set_fd( ssl, fd );
    if ( comm_mode == CommMode::Server )
        SSL_set_accept_state( ssl );
    else
        SSL_set_connect_state( ssl );

    // * SSL_MODE_ENABLE_PARTIAL_WRITE is not used: openssl does the job for us
    // * SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER is here because allow_transfer_ownership may be false in send args
    SSL_set_mode( ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER );

    if ( pref_ciph && SSL_set_cipher_list( ssl, pref_ciph ) != 1 ) {
        ssl_error( ERR_get_error(), "SSL_set_cipher_list:" );
        abort();
    }
}


TlsConnection::TlsConnection( VtableOnly vo ) : TcpConnection( vo ) {
}

TlsConnection::~TlsConnection() {
    if ( ssl )
        SSL_free( ssl );
}

void TlsConnection::send( const char **data, size_t size, size_t rese, bool allow_transfer_ownership ) {
    if ( size == 0 || has_error || want_close_fd || want_a_shutdown || next_del )
        return;

    if ( waiting_for_connection || ! waiting_sends.empty() || to_redo != ToRedo::Nothing )
        return add_send_item( data, size, rese, allow_transfer_ownership );

    // try a write
    while ( true ) {
        int ruff = SSL_write( ssl, *data, size );

        switch ( SSL_get_error( ssl, ruff ) ) {
        case SSL_ERROR_NONE:
            // no partial write => nothing to do
            break;
        case SSL_ERROR_WANT_READ:
            add_send_item( data, size, rese, allow_transfer_ownership );

            wanted_direction = Direction::Inp;
            to_redo          = ToRedo::Write;
            return;
        case SSL_ERROR_WANT_WRITE:
            add_send_item( data, size, rese, allow_transfer_ownership );

            wanted_direction = Direction::Out;
            to_redo          = ToRedo::Write;
            return;
        case SSL_ERROR_ZERO_RETURN:
            return on_rd_hup();
        case SSL_ERROR_SYSCALL:
            if ( errno == EINTR )
                continue;
            return sys_error();
        default:
            return ssl_error( ERR_get_error() );
        }
        break;
    }
}

void TlsConnection::close() {
    if ( has_error || want_a_shutdown )
        return;
    want_a_shutdown = true;

    // we can do the shutdown now ?
    if ( waiting_for_connection == false && waiting_sends.empty() ) {
        started_a_shutdown = true;
        _shutdown();
    }
}

void TlsConnection::ssl_error( unsigned long err, const char *context ) {
    if( const char *str = ERR_reason_error_string( err ) )
        ssl_error( str, context );
    else {
        std::string s = "Ssl error: " + std::to_string( err );
        ssl_error( s.c_str(), context );
    }
}

void TlsConnection::ssl_error( const char *msg, const char *context ) {
    ev_loop->err( msg, context );
    has_error = true;
    close_fd();
}

void TlsConnection::on_inp() {
    if ( has_error )
        return;

    if ( to_redo != ToRedo::Nothing ) {
        if ( wanted_direction != Direction::Inp ) {
            has_waiting_inp_data = true;
            return;
        }

        _redo();
        if ( to_redo != ToRedo::Nothing || has_error )
            return;

        // try to send old stuff
        _write();
        if ( to_redo != ToRedo::Nothing || has_error )
            return;
    }

    _read();
    if ( to_redo != ToRedo::Nothing || has_error )
        return;

    // try to send stuff
    _write();
    if ( to_redo != ToRedo::Nothing || has_error )
        return;

    // need to start a shutdown ?
    if ( want_a_shutdown && ! started_a_shutdown && waiting_sends.empty() ) {
        started_a_shutdown = true;
        _shutdown();
    }
}

void TlsConnection::on_out() {
    if ( has_error )
        return;

    // => connection is now available
    if ( waiting_for_connection ) {
        waiting_for_connection = false;

        if ( comm_mode == CommMode::Client ) {
            _handshake();
            if ( to_redo != ToRedo::Nothing || has_error )
                return;
        }
    }

    if ( to_redo != ToRedo::Nothing ) {
        if ( wanted_direction != Direction::Out )
            return;

        _redo();
        if ( to_redo != ToRedo::Nothing || has_error )
            return;
    }

    _write();
    if ( to_redo != ToRedo::Nothing )
        return;

    // if we had inputs when waiting for out sig, it's time now to use the data
    if ( has_waiting_inp_data ) {
        has_waiting_inp_data = false;
        on_inp();
        if ( to_redo != ToRedo::Nothing || has_error )
            return;
    }

    // need to start a shutdown ?
    if ( want_a_shutdown && ! started_a_shutdown && waiting_sends.empty() ) {
        started_a_shutdown = true;
        _shutdown();
    }
}

void TlsConnection::_redo() {
    ToRedo tr = to_redo;
    to_redo = ToRedo::Nothing;

    switch ( tr ) {
    case ToRedo::Nothing:   return;
    case ToRedo::Handshake: return _handshake();
    case ToRedo::Read:      return _read();
    case ToRedo::Write:     return _write();
    case ToRedo::Shutdown:  return _shutdown();
    }
}

void TlsConnection::_handshake() {
    while ( true ) {
        int ruff = SSL_do_handshake( ssl );

        switch ( SSL_get_error( ssl, ruff ) ) {
        case SSL_ERROR_NONE:
            break;
        case SSL_ERROR_WANT_READ:
            wanted_direction = Direction::Inp;
            to_redo          = ToRedo::Handshake;
            return;
        case SSL_ERROR_WANT_WRITE:
            wanted_direction = Direction::Out;
            to_redo          = ToRedo::Handshake;
            return;
        case SSL_ERROR_ZERO_RETURN:
            return on_rd_hup();
        case SSL_ERROR_SYSCALL:
            if ( errno == EINTR )
                continue;
            if ( errno == 0 )
                return ssl_error( "SSL handshake failure (not a SSL endpoint ?)" );
            ev_loop->err( "Ssl error syscall (in SSL_do_handshake): ", strerror( errno ) );
            return sys_error();
        default:
            return ssl_error( ERR_get_error() );
        }

        break;
    }
}

void TlsConnection::_shutdown() {
    // std::cout << fcntl(fd, F_GETFD) << ", " << errno << std::endl;

    while ( true ) {
        int ruff = SSL_shutdown( ssl );

        switch ( SSL_get_error( ssl, ruff ) ) {
        case SSL_ERROR_NONE:
            close_fd();
            break;
        case SSL_ERROR_WANT_READ:
            wanted_direction = Direction::Inp;
            to_redo          = ToRedo::Shutdown;
            return;
        case SSL_ERROR_WANT_WRITE:
            wanted_direction = Direction::Out;
            to_redo          = ToRedo::Shutdown;
            return;
        case SSL_ERROR_ZERO_RETURN:
            return on_rd_hup();
        case SSL_ERROR_SYSCALL:
            if ( errno == EINTR )
                continue;
            if ( errno == 0 )
                break;
            return sys_error();
        default:
            want_close_fd = true;
            return ssl_error( ERR_get_error() );
        }

        break;
    }
}

void TlsConnection::_write() {
    while ( ! waiting_sends.empty() ) {
        SendItem &si = waiting_sends.front();
        int ruff = SSL_write( ssl, si.data, si.size );

        switch ( SSL_get_error( ssl, ruff ) ) {
        case SSL_ERROR_NONE:
            // no partial write => we can completely remove si
            msg_free( si.allo );
            waiting_sends.pop_front();
            break;
        case SSL_ERROR_WANT_READ:
            wanted_direction = Direction::Inp;
            to_redo          = ToRedo::Write;
            return;
        case SSL_ERROR_WANT_WRITE:
            wanted_direction = Direction::Out;
            to_redo          = ToRedo::Write;
            return;
        case SSL_ERROR_ZERO_RETURN:
            return on_rd_hup();
        case SSL_ERROR_SYSCALL:
            if ( errno == EINTR )
                continue;
            ev_loop->err( "Ssl error syscall (in SSL_write): ", strerror( errno ) );
            return sys_error();
        default:
            return ssl_error( ERR_get_error() );
        }
    }
}

void TlsConnection::_read() {
    while ( true ) {
        // try to find the message length (based on ciphered string)
        unsigned buff_size, max_buff_size = 1u << 20;
        while ( true ) {
            if ( ioctl( fd, FIONREAD, &buff_size ) == -1 ) {
                if ( errno == EINTR )
                    continue;
                ev_loop->log( "ioctl FIONREAD: ", strerror( errno ) );
                buff_size = 8192;
            } else if ( buff_size > max_buff_size )
                buff_size = max_buff_size;
            break;
        }

        // if no available inp_buffer or if size is not high enough, create a new one
        size_t off = offset_parse(), rese = off + buff_size;
        if ( ! inp_buffer ) {
            inp_buffer = (char *)msg_alloc( rese );
            inp_buffer_size = rese - off;
        } else if ( inp_buffer_size < buff_size ) {
            msg_free( inp_buffer );
            inp_buffer = (char *)msg_alloc( rese );
            inp_buffer_size = rese - off;
        }

        // read some data
        while ( true ) {
            int ruff = SSL_read( ssl, off + inp_buffer, inp_buffer_size );

            // error/OK cases
            switch ( SSL_get_error( ssl, ruff ) ) {
            case SSL_ERROR_NONE:
                //            if ( ! handshake_done ) {
                //                handshake_done = true;
                //                if ( ! check_X509() )
                //                    break;
                //            }
                if ( want_close_fd == false && next_del == 0 && want_a_shutdown == false )
                    parse( &inp_buffer, ruff, inp_buffer_size );
                break;
            case SSL_ERROR_WANT_READ:
                wanted_direction = Direction::Inp;
                to_redo          = ToRedo::Read;
                return;
            case SSL_ERROR_WANT_WRITE:
                wanted_direction = Direction::Out;
                to_redo          = ToRedo::Read;
                return;
            case SSL_ERROR_ZERO_RETURN:
                return on_rd_hup(); // The TLS/SSL connection has been closed
            case SSL_ERROR_SYSCALL:
                if ( errno == EINTR )
                    continue;
                if ( errno == 0 )
                    return;
                ev_loop->err( "Ssl error syscall (in SSL_read): ", strerror( errno ) );
                return sys_error();
            default:
                return ssl_error( ERR_get_error() );
            }

            break;
        }
    }
}


//bool SslConnection::check_X509() {
//    // Step 1: verify a server certifcate was presented during negotiation
//    // https://www.openssl.org/docs/ssl/SSL_get_peer_certificate.html
//    X509 *cert = SSL_get_peer_certificate( ssl );
//    if ( ! cert ) {
//        ssl_error( ERR_get_error() ); // X509_V_ERR_APPLICATION_VERIFICATION
//        return false;
//    }
//    X509_free( cert ); // Free immediately

//    // Step 2: verify the result of chain verifcation
//    // http://www.openssl.org/docs/ssl/SSL_get_verify_result.html
//    // Error codes: http://www.openssl.org/docs/apps/verify.html
//    if ( long res = SSL_get_verify_result( ssl ) ) {
//        /* Hack a code into print_error_string. */
//        ssl_error( res );
//        return false;
//    }

//    // seems to be OK
//    return true;
//}

} // namespace Evel
