#pragma once

#include "Timer.h"

namespace Evel {
class UdpConnectionFactory;

/**
*/
class UdpConnFactTimeoutsTimer : public Timer {
public:
    UdpConnFactTimeoutsTimer( UdpConnectionFactory *ucf, double freq );

protected:
    virtual void timeout( unsigned ) override;

    UdpConnectionFactory *ucf;
};

} // namespace Evel
