#include <openssl/crypto.h>
#include <openssl/engine.h>
#include <openssl/conf.h>
#include "SslCtx.h"
#include <iostream>
#include <vector>
#include <string>
#include <mutex>

//// nsmake lib_name crypto
//// nsmake lib_name ssl

/**
  Add session resumption feature - may be later version
  Renegotiation not yet handled - may be later version

 */
namespace Evel {

namespace {
    static std::vector<std::mutex> mutexes;

    static void locking_callback( int mode, int type, const char *file, int line ) {
        if ( mode & CRYPTO_LOCK )
            mutexes[ type ].lock();
        else
            mutexes[ type ].unlock();
    }

    struct InitSsl {
        InitSsl() {
            SSL_library_init();
            SSL_load_error_strings();
            ERR_load_BIO_strings();
            OpenSSL_add_all_algorithms(); // load & register all cryptos, etc.
            ERR_load_crypto_strings();

            std::vector<std::mutex> m( CRYPTO_num_locks() );
            mutexes.swap( m );
            CRYPTO_set_locking_callback( locking_callback );
        }

        ~InitSsl() {
            ERR_remove_state(0);
            ENGINE_cleanup();
            CONF_modules_unload(1);
            ERR_free_strings();
            EVP_cleanup();
            sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
            CRYPTO_cleanup_all_ex_data();
        }

    };

    InitSsl &init_ssl_if_necessary() {
        static InitSsl res;
        return res;
    }
}

SslCtx::SslCtx( Method method_name, const char *cert_file, const char *key_file, std::function<void(const char *)> on_err ): server( true ) {
    init_ssl_if_necessary();

    // method
    const SSL_METHOD *method = 0;
    switch ( method_name ) {
    case Method::TLSv12: method = TLSv1_2_server_method(); break;
    case Method::SSLv23: method = SSLv23_server_method(); break;
    case Method::DTLSv1: method = DTLSv1_server_method(); break;
    }
    if ( ! method ) {
        ssl_error( ERR_get_error(), "..._server_method()", on_err );
        ctx = 0;
        return;
    }

    // server ctx
    ctx = SSL_CTX_new( method ); // create new context from method
    if ( ! ctx ) {
        ssl_error( ERR_get_error(), "SSL_CTX_new", on_err );
        return;
    }

    // Remove the most egregious. Because SSLv2 and SSLv3 have been
    // removed, a TLSv1.0 handshake is used. The client accepts TLSv1.0
    // and above. An added benefit of TLS 1.0 and above are TLS
    // extensions like Server Name Indicatior (SNI).
    SSL_CTX_set_options( ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION );

    // set the local certificate from cert_file
    if ( SSL_CTX_use_certificate_file( ctx, cert_file, SSL_FILETYPE_PEM ) <= 0 ) {
        ssl_error( ERR_get_error(), "SSL_CTX_use_certificate_file", on_err );
        SSL_CTX_free( ctx );
        ctx = 0;
        return;
    }

    // set the private key from key_file (may be the same as CertFile)
    if ( SSL_CTX_use_PrivateKey_file( ctx, key_file, SSL_FILETYPE_PEM ) <= 0 ) {
        ssl_error( ERR_get_error(), "SSL_CTX_use_PrivateKey_file", on_err );
        SSL_CTX_free( ctx );
        ctx = 0;
        return;
    }

    // verify private key
    if ( not SSL_CTX_check_private_key( ctx ) ) {
        if ( on_err )
            on_err( "Private key does not match the public certificate" );
        else
            std::cerr << "Private key does not match the public certificate" << std::endl;
        SSL_CTX_free( ctx );
        ctx = 0;
        return;
    }
}

SslCtx::SslCtx( Method method_name, bool check_cert, std::function<void(const char *)> on_err ): server( false ) {
    init_ssl_if_necessary();

    // method
    const SSL_METHOD *method = 0;
    switch ( method_name ) {
    case Method::TLSv12: method = TLSv1_2_client_method(); break;
    case Method::SSLv23: method = SSLv23_client_method(); break;
    case Method::DTLSv1: method = DTLSv1_client_method(); break;
    }
    if ( ! method ) {
        ssl_error( ERR_get_error(), "..._client_method()", on_err );
        ctx = 0;
        return;
    }

    // ctx
    ctx = SSL_CTX_new( method ); // create new context from method
    if ( ! ctx ) {
        ssl_error( ERR_get_error(), "SSL_CTX_new", on_err );
        return;
    }

    //
    if ( check_cert )
        SSL_CTX_set_verify( ctx, SSL_VERIFY_PEER, NULL ); // verify_callback

    // Remove the most egregious. Because SSLv2 and SSLv3 have been
    // removed, a TLSv1.0 handshake is used. The client accepts TLSv1.0
    // and above. An added benefit of TLS 1.0 and above are TLS
    // extensions like Server Name Indicatior (SNI).
    SSL_CTX_set_options( ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION );
}

SslCtx::SslCtx() {
    ctx = 0;
}

SslCtx::~SslCtx() {
    if ( ctx )
        SSL_CTX_free( ctx );
}

void SslCtx::ssl_error( unsigned long err, const char *label, std::function<void(const char *)> on_err ) {
    std::string s = std::string( label ) + " failed: ";

    if ( const char *str = ERR_reason_error_string( err ) )
        s += str;
    else
        s += std::to_string( err );

    if ( on_err )
        on_err( s.c_str() );
    else
        std::cerr << s << std::endl;
}


SslCtx::operator bool() const {
    return ctx;
}

} // namespace Evel

