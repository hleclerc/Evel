#pragma once

#include "InetAddress.h"

namespace Evel {
class UdpConnectionFactory;

/**
*/
class UdpConnection {
public:
    UdpConnection( UdpConnectionFactory *ucf, const InetAddress &addr );
    virtual ~UdpConnection();

    virtual void          close            ();
    virtual void          del              ();

    void                  send             ( const char *data );               ///< simplified version. data will be copied if send has actually to be postponed
    void                  send             ( const char *data, size_t size ); ///< simplified version. data will be copied if send has actually to be postponed
    virtual void          send             ( const char **data, size_t size, bool allow_transfer_ownership = true ); ///< if transfer_ownership is allowed, send may take ownership of *data, in which case, *data is changed to null and will be freed using free()

    UdpConnectionFactory *ucf;
    InetAddress           addr;

protected:
    friend class UdpConnectionFactory;

    virtual void          parse            ( char **data, size_t size ) = 0;
    virtual void          _parse           ( char **data, size_t size ); ///< function called by UdpConnectionFactory
    void                  send_raw         ( const char **data, size_t size, bool allow_transfer_ownership );
    virtual bool          send_avail_at_beg() const; ///< true if data can be sent at the beginning (not the case for DTLS for instance)

    virtual void          sys_error        ( const char *ctx = 0 ); ///< I/O error

    virtual void         *msg_alloc        ( size_t size );
    virtual void          msg_free         ( const void *ptr );

    UdpConnection        *next_del;
    bool                  has_error;
};

} // namespace Evel
