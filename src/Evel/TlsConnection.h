#pragma once

#include "TcpConnection.h"
#include "InetAddress.h"
#include <openssl/ssl.h>
#include <vector>
#include <deque>

// based on https://hynek.me/articles/hardening-your-web-servers-ssl-ciphers/
#ifndef PREF_CIPHER_EVEL
#define PREF_CIPHER_EVEL "ECDH+AESGCM:DH+AESGCM:ECDH+AES256:DH+AES256:ECDH+AES128:DH+AES:RSA+AESGCM:RSA+AES:!aNULL:!MD5:!DSS"
#endif // PREF_CIPHER_EVEL
// HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS

namespace Evel {

/**
  Prop: if failure on output, we wait for the next output to redo the same stuff. If input, we mark
*/
class TlsConnection : public TcpConnection {
public:
    TlsConnection( SSL_CTX *ssl_ctx, const InetAddress &addr, const char *pref_ciph = PREF_CIPHER_EVEL ); ///< client mode
    TlsConnection( SSL_CTX *ssl_ctx, int accepting_fd, const char *pref_ciph = PREF_CIPHER_EVEL );        ///< accepting (server) mode
    TlsConnection( VtableOnly ); ///< a constructor that does not assign any attribute (else than the vtable). Permits to do a new( ptr ) T to change _only_ the vtable (underlying type)
    ~TlsConnection();

    using TcpConnection::send;
    virtual void  send          ( const char **data, size_t size, size_t rese = 0, bool allow_transfer_ownership = true ); ///< if transfer_ownership is allowed, send may take ownership of *data, in which case, *data is changed to null and will be freed using free(). return value is relevant only if it's for a retry

    virtual void  close         ();               ///< delete this after all the outputs has been seen. Initiate a SSL_shutdown

    bool          allow_self_signed_certificates; ///< false by default
    double        read_timeout;                   ///< in seconds (5 by default)

protected:
    /// states are mainly here for optimization (to avoid worthless system calls), because openssl seems to be able to somewhat manage states by itself
    enum class ToRedo    : char { Nothing = 0, Handshake, Read, Write, Shutdown };
    enum class CommMode  : char { Server, Client };
    enum class Direction : char { Inp, Out };

    virtual void  ssl_error     ( unsigned long err, const char *context = 0 );
    virtual void  ssl_error     ( const char *msg, const char *context = 0 );

    virtual void  on_inp        () override;
    virtual void  on_out        () override;

    void          call_SSL_write();
    void          call_SSL_read ();
    bool          check_X509    ();

    void          _init         ( SSL_CTX *ssl_ctx, const char *pref_ciph );
    void          _handshake    ();
    void          _shutdown     ();
    void          _write        ();
    void          _read         ();
    void          _redo         ();

    SSL          *ssl;
    ToRedo        to_redo;
    Direction     redo_dir;             ///< what ToRedo waits
    CommMode      comm_mode;
    bool          handshake_done;
    bool          want_a_shutdown;      ///<
    bool          started_a_shutdown;   ///<
    bool          has_waiting_inp_data; ///< true if received a signal for a non waiting direction
};

} // namespace Evel
