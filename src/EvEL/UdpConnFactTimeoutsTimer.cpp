#include "UdpConnFactTimeoutsTimer.h"
#include "UdpConnectionFactory.h"
#include "EvLoop.h"

namespace Evel {

UdpConnFactTimeoutsTimer::UdpConnFactTimeoutsTimer( UdpConnectionFactory *ucf, double freq ) : Timer( freq, false ), ucf( ucf ) {
}

void UdpConnFactTimeoutsTimer::timeout( unsigned nb_expires ) {
    while( nb_expires-- ) {
        ucf->timeout_list.shift( [&]( UdpConnection *conn ) {
            conn->on_timeout();
        } );
    }
}

} // namespace Evel
