#include "TimeoutsTimer.h"
#include "EvLoop.h"

namespace Evel {

TimeoutsTimer::TimeoutsTimer( double freq ) : Timer( freq, false ) {
}

void TimeoutsTimer::timeout( unsigned nb_expires ) {
    while( nb_expires-- ) {
        ev_loop->timeout_list.shift( [&]( TimeoutEvent *ev ) {
            ev->on_timeout();
        } );
    }
}

} // namespace Evel
