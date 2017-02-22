#pragma once

#include "UdpConnectionFactory.h"
#include <functional>

namespace Evel {

/// specialisation that works with std::functions
class UdpConnectionFactory_WF : public UdpConnectionFactory {
public:
    using TF_factory  = std::function<UdpConnection *( UdpConnectionFactory_WF *ucf, const InetAddress &src )>;
    using TF_on_close = std::function<void( UdpConnectionFactory_WF *ucf )>;

    UdpConnectionFactory_WF( TF_factory &&f_factory = {} );
    ~UdpConnectionFactory_WF();

    TF_factory             f_factory;
    TF_on_close            f_on_close;

protected:
    virtual UdpConnection *factory( const InetAddress &src );
};

} // namespace Evel
