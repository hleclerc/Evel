#pragma once

#include "UdpConnection.h"
#include <functional>

namespace Evel {

/**
*/
class UdpConnection_WF : public UdpConnection {
public:
    using TF_parse  = std::function<void( UdpConnection_WF *conn, char **data, size_t size )>;
    using TF_on_del = std::function<void( UdpConnection_WF *conn )>;

    UdpConnection_WF( UdpConnectionFactory *ucf, const InetAddress &addr, TF_parse &&f_parse = {} );
    ~UdpConnection_WF();

    TF_parse              f_parse;
    TF_on_del             f_on_del;

protected:
    virtual void          parse( char **data, size_t size );
};

} // namespace Evel
