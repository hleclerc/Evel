#pragma once

#include "InetAddress.h"
#include "Event.h"
#include <deque>


namespace Evel {

/**
*/
class UdpSocket : public Event {
public:
    UdpSocket( unsigned inp_buf_size = 2048, bool need_wait = true );
    ~UdpSocket();

    void          bind            ( int port );
    void          send            ( const InetAddress &dst, const char *data );  ///< simplified version. data will be copied if send has actually to be postponed
    void          send            ( const InetAddress &dst, const char *data, size_t size ); ///< simplified version. data will be copied if send has actually to be postponed
    void          send            ( const InetAddress &dst, char **data, size_t size, bool allow_transfer_ownership = true ); ///< if transfer_ownership is allowed, send may take ownership of *data, in which case, *data is changed to null and will be freed using free(). return value is relevant only if it's for a retry
    virtual void  send            ( const InetAddress &dst, const char **data, size_t size, bool allow_transfer_ownership = true ); ///< if transfer_ownership is allowed, send may take ownership of *data, in which case, *data is changed to null and will be freed using free(). return value is relevant only if it's for a retry
    unsigned      get_inp_buf_size() const { return inp_buf_size; }


protected:
    struct SendItem { InetAddress dst; const char *data; size_t size; };
    using DC = std::deque<SendItem>;
    friend class UdpConnection;

    virtual void  on_bind_error   ( const char *msg );
    virtual void *allocate        ( size_t inp_buf_size ); ///< to describe how to allocate `buffer` memory
    virtual void  parse           ( const InetAddress &src, char **data, unsigned size ) = 0; ///< data can be modified to become owned (by default, buffer comes from a malloc, modifiable with `allocate`)
    void          send_raw        ( const InetAddress &dst, const char **data, size_t size, bool allow_transfer_ownership ); ///< if transfer_ownership is allowed, send may take ownership of *data, in which case, *data is changed to null and will be freed using free()

    virtual void  on_inp          () override;
    virtual void  on_out          () override;
    virtual bool  out_are_sent    () const override;

    void          add_send_item   ( const InetAddress &dst, const char **data, size_t size, bool allow_transfer_ownership );

    virtual void *msg_alloc       ( size_t size );
    virtual void  msg_free        ( const void *ptr );

    DC            waiting_sends;
    unsigned      inp_buf_size; ///< size 
    char         *buffer;       ///< room to read data
};


} // namespace Evel
