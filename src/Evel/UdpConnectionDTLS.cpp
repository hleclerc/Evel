#include "UdpConnectionFactory.h"
#include "UdpConnectionDTLS.h"
#include "EvLoop.h"
#include <openssl/err.h>

namespace Evel {

UdpConnectionDTLS::UdpConnectionDTLS( SSL_CTX *ctx, bool server, UdpConnectionFactory *ucf, const InetAddress &addr ) : UdpConnection( ucf, addr ), deciphered_input( 0 ), ciphered_output( 0 ), want_close( false ) {
    // create SSL
    if ( ! ( ssl = SSL_new( ctx ) ) ) {
        std::cerr << "Cannot create new SSL." << std::endl;
        return;
    }

    if ( server )
        SSL_set_accept_state( ssl );
    else
        SSL_set_connect_state( ssl );

    // bios
    if ( ! ( inp_bio = BIO_new( BIO_s_mem() ) ) ) { std::cerr << "Cannot allocate inp bio." << std::endl; SSL_free( ssl ); ssl = NULL; return; }
    if ( ! ( out_bio = BIO_new( BIO_s_mem() ) ) ) { std::cerr << "Cannot allocate out bio." << std::endl; SSL_free( ssl ); ssl = NULL; return; }
    BIO_set_mem_eof_return( inp_bio, -1 ); // to allow asynchronous IO
    BIO_set_mem_eof_return( out_bio, -1 ); // to allow asynchronous IO
    SSL_set_bio( ssl, inp_bio, out_bio );

    if ( ! server ) {
        while ( true ) {
            int ruff = SSL_do_handshake( ssl );

            switch ( SSL_get_error( ssl, ruff ) ) {
            case SSL_ERROR_WANT_READ:
                ucf->add_timeout( this, 1.0 );
            case SSL_ERROR_NONE:
                flush_out_bio();
                break;
            case SSL_ERROR_WANT_WRITE:
                flush_out_bio();
                continue;
            case SSL_ERROR_SYSCALL:
                if ( errno == EINTR )
                    continue;
                sys_error( "SSL_do_handshake" ); // ! won't call the surfefined version
                return;
            default:
                ssl_error( "SSL_do_handshake" ); // ! won't call the surfefined version
                return;
            }
            break;
        }
    }

    done_dec_wait_out = false;
    ucf->inc_nb_live_conn();
}

UdpConnectionDTLS::~UdpConnectionDTLS() {
    if ( ! done_dec_wait_out ) ucf->dec_nb_live_conn();
    if ( deciphered_input    ) msg_free( deciphered_input );
    if ( ciphered_output     ) msg_free( ciphered_output  );
    if ( ssl                 ) SSL_free( ssl );
}

void UdpConnectionDTLS::send( const char **data, size_t size, bool allow_transfer_ownership ) {
    if ( want_close || next_del || ! ssl )
        return;

    while ( true ) {
        int err = SSL_get_error( ssl, SSL_write( ssl, *data, size ) );
        switch ( err ) {
        case SSL_ERROR_NONE:
            flush_out_bio();
            break;
        case SSL_ERROR_WANT_READ:
            store( data, size, allow_transfer_ownership );
            ucf->add_timeout( this, 1.0 );
            flush_out_bio();
            break;
        case SSL_ERROR_WANT_WRITE:
            flush_out_bio();
            continue;
        case SSL_ERROR_SYSCALL:
            if ( errno == EINTR )
                continue;
            return sys_error( "SSL_write" );
        default:
            PLE( err );
            return ssl_error( "SSL_write" );
        }
        break;
    }
}

void UdpConnectionDTLS::close() {
    if ( SSL_is_init_finished( ssl ) && waiting_sends.empty() )
        del();
    else
        want_close = true;
}

bool UdpConnectionDTLS::send_avail_at_beg() const {
    return false;
}

void UdpConnectionDTLS::store( const char **data, size_t size, bool allow_transfer_ownership ) {
    if ( ! ssl || ! size )
        return;
    if ( allow_transfer_ownership ) {
        waiting_sends.emplace_back( Send{ *data, size } );
        data = 0;
    } else {
        char *n_data = (char *)msg_alloc( size );
        memcpy( n_data, *data, size );
        waiting_sends.emplace_back( Send{ n_data, size } );
    }
}

void UdpConnectionDTLS::flush_sends() {
    if ( ! ssl )
        return;

    while ( ! waiting_sends.empty() ) {
        Send &p = waiting_sends.front();
        if ( SSL_write( ssl, p.data, p.size ) <= 0 )
            break;
        msg_free( const_cast<char *>( p.data ) );
        waiting_sends.pop_front();
        flush_out_bio();
    }
}

void UdpConnectionDTLS::_parse( char **data, size_t size ) {
    ucf->rem_timeout( this );
    if ( ! ssl )
        return;
    ssize_t sent = BIO_write( inp_bio, *data, size );
    if ( sent != size )
        ucf->ev_loop->err( "Pb with BIO_write" );

    //
    if ( ! SSL_is_init_finished( ssl ) ) {
        // continue handshake (client or server)
        while ( true ) {
            switch ( SSL_get_error( ssl, SSL_do_handshake( ssl ) ) ) {
            case SSL_ERROR_NONE:
                flush_out_bio();
                break;
            case SSL_ERROR_WANT_READ:
                ucf->add_timeout( this, 1.0 );
                flush_out_bio();
                return;
            case SSL_ERROR_WANT_WRITE:
                flush_out_bio();
                continue;
            case SSL_ERROR_SYSCALL:
                if ( errno == EINTR )
                    continue;
                sys_error( "SSL_do_handshake" );
                return;
            default:
                ssl_error( "SSL_do_handshake" );
                return;
            }
            break;
        }

        if ( SSL_is_init_finished( ssl ) ) {
            flush_sends();
            on_rdy();
            done_dec_wait_out = true;
            if ( want_close )
                del();
            ucf->dec_nb_live_conn();
        }
        return;
    }

    // read inp_bio
    if ( ! deciphered_input )
        deciphered_input = (char *)msg_alloc( ucf->get_inp_buf_size() );
    while ( true ) {
        ssize_t read = SSL_read( ssl, deciphered_input, ucf->get_inp_buf_size() );

        switch ( SSL_get_error( ssl, read ) ) {
        case SSL_ERROR_NONE:
            parse( &deciphered_input, read );
            flush_out_bio();
            if ( next_del || ( want_close && SSL_is_init_finished( ssl ) ) )
                return;
            break;
        case SSL_ERROR_WANT_READ:
            ucf->add_timeout( this, 1.0 );
            flush_out_bio();
            return;
        case SSL_ERROR_WANT_WRITE:
            flush_out_bio();
            continue;
        case SSL_ERROR_SYSCALL:
            if ( errno == EINTR )
                continue;
            sys_error( "SSL_read" );
            return;
        default:
            ssl_error( "SSL_read" );
            return;
        }
    }
}

void UdpConnectionDTLS::on_rdy() {
}

double UdpConnectionDTLS::timeout_read() const {
    return 10.0;
}

void UdpConnectionDTLS::flush_out_bio() {
    if ( ! ssl )
        return;

    // make room to receive the data
    unsigned size = std::max( ucf->get_inp_buf_size() + 96u, 1012u );
    if ( ! ciphered_output )
        ciphered_output = (char *)malloc( size );

    // read bio
    while ( true ) {
        int read = BIO_read( out_bio, ciphered_output, size );

        switch ( int err = SSL_get_error( ssl, read ) ) {
        // send the content
        case SSL_ERROR_NONE:
            if ( read > 0 )
                send_raw( (const char **)&ciphered_output, read, true );
            break;
        case SSL_ERROR_WANT_READ:
            ucf->add_timeout( this, 1.0 );
            return;
        case SSL_ERROR_SYSCALL:
            if ( errno == EINTR )
                continue;
            if ( errno == 0 )
                break;
            sys_error( "BIO_read" );
            return;
        default:
            PLE( err );
            ssl_error( "BIO_read" );
            return;
        }
        break;
    }
}

void UdpConnectionDTLS::ssl_error( const char *ctx ) {
    if ( ctx ) std::cerr << ctx << ": ";
    ERR_print_errors_fp( stderr );
    del();
}

} // namespace Evel
