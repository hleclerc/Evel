#pragma once

#include "InetAddress.h"
#include "Event.h"
#include <vector>
#include <deque>

namespace Evel {

/**
*/
class TcpConnection : public Event {
public:
    TcpConnection( const InetAddress &addr ); ///< client mode
    TcpConnection( int accepting_fd );        ///< accepting (server) mode
    TcpConnection( VtableOnly );              ///< a constructor that does not assign any attribute (else than the vtable). Permits to do a new( ptr ) T to change _only_ the vtable (underlying type)
    ~TcpConnection();

    void             send         ( const char *data );  ///< simplified version. data will be copied if send has actually to be postponed
    void             send         ( const char *data, size_t size ); ///< simplified version. data will be copied if send has actually to be postponed
    virtual void     send         ( const char **data, size_t size, size_t rese = 0, bool allow_transfer_ownership = true ); ///< if transfer_ownership is allowed, send may take ownership of *data, in which case, *data is changed to null and will be freed using free(). return value is relevant only if it's for a retry

protected:
    struct SendItem { const char *data, *allo; size_t size, rese; };
    using VIOVEC = std::vector<iovec>;
    using DC = std::deque<SendItem>;

    virtual void     parse        ( char **data, size_t size, size_t rese ) = 0; ///< data can be modified to become owned (by default, buffer comes from a malloc, modifiable with `allocate`)
    virtual size_t   offset_parse () const; ///< enable to have a data header.

    virtual void     on_inp       () override;
    virtual void     on_out       () override;
    virtual bool     out_are_sent () const override;

    void             add_send_item( const char **data, size_t size, size_t rese, bool allow_transfer_ownership );

    virtual void    *msg_alloc    ( size_t size );
    virtual void     msg_free     ( const void *ptr );

    bool             waiting_for_connection;
    unsigned         inp_buffer_size; ///< room reserved in buffer
    char            *inp_buffer;      ///< room to read data
    DC               waiting_sends;
    VIOVEC           viovec;
};

} // namespace Evel
