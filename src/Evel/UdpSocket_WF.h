#pragma once

#include "UdpSocket.h"
#include <functional>

namespace Evel {

/**
*/
class UdpSocket_WF : public UdpSocket {
public:
    using TF_rdy           = std::function<void ( UdpSocket_WF *socket )>;
    using TF_parse         = std::function<void ( UdpSocket_WF *socket, const InetAddress &src, char **data, unsigned size )>;
    using TF_on_bind_error = std::function<void ( UdpSocket_WF *socket, const char *msg )>;
    using TF_allocate      = std::function<void*( UdpSocket_WF *socket, size_t size )>;
    using TF_close         = std::function<void ( UdpSocket_WF *socket )>;

    /// if f_parse is not defined wait_for_inp_at_beg() will be false
    using UdpSocket::UdpSocket;
    ~UdpSocket_WF();

    TF_rdy           f_rdy;
    TF_parse         f_parse;
    TF_close         f_close;
    TF_allocate      f_allocate;
    TF_on_bind_error f_on_bind_error;

protected:
    virtual void     on_rdy                ();
    virtual void     parse              ( const InetAddress &src, char **data, unsigned size );
    virtual void    *allocate           ( size_t inp_buf_size );
    virtual void     on_bind_error      ( const char *msg );
};


} // namespace Evel
