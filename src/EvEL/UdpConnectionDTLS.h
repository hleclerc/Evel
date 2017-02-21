#pragma once

#include "UdpConnection.h"
#include <openssl/ssl.h>
#include <deque>

namespace Evel {

/**
*/
class UdpConnectionDTLS : public UdpConnection {
public:
    UdpConnectionDTLS( SSL_CTX *ctx, bool server, UdpConnectionFactory *ucf, const InetAddress &addr );
    ~UdpConnectionDTLS();

    using UdpConnection::send;
    virtual void  send             ( const char **data, size_t size, bool allow_transfer_ownership = true ) override; ///< if transfer_ownership is allowed, send may take ownership of *data, in which case, *data is changed to null and will be freed using free()

    virtual void  close            () override;

protected:
    struct Send { const char *data; size_t size; };
    using DC = std::deque<Send>;

    virtual bool  send_avail_at_beg() const override; ///< true if data can be sent at the beginning (not the case for DTLS for instance)
    virtual void  _parse           ( char **data, size_t size ) override; ///< function called by UdpConnectionFactory
    virtual void  on_rdy           (); ///< called when the connection is up and running

    void          store            ( const char **data, size_t size, bool allow_transfer_ownership ); ///< if transfer_ownership is allowed, send may take ownership of *data, in which case, *data is changed to null and will be freed using free()
    void          flush_sends      ();
    void          flush_out_bio    ();
    virtual void  ssl_error        ( const char *ctx = 0 ); ///<

    SSL          *ssl;
    BIO          *inp_bio;
    BIO          *out_bio;
    DC            waiting_sends;
    char         *ciphered_output;
    char         *deciphered_input;
    bool          done_dec_wait_out;
    bool          want_close;
};

} // namespace Evel
