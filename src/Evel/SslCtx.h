#pragma once

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <functional>

namespace Evel {

/**
*/
class SslCtx {
public:
    enum class Method {
        SSLv23,
        TLSv12,
        DTLSv1
    };

    SslCtx( Method method_name, const char *cert_file, const char *key_file, std::function<void(const char *)> on_err = {} ); ///< for a server connection
    SslCtx( Method method_name, bool check_cert = true, std::function<void(const char *)> on_err = {} ); ///< for a client connection
    SslCtx(); ///< for no connection
    ~SslCtx();

    operator bool     () const; ///< true if ok
    operator SSL_CTX* () { return ctx; }

    void     ssl_error( unsigned long err, const char *label, std::function<void(const char *)> on_err );

    SSL_CTX *ctx;
    bool     server;
};

} // namespace Evel
