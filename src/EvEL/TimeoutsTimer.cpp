#include "TimeoutsTimer.h"
#include "EvLoop.h"

namespace Evel {

TimeoutsTimer::TimeoutsTimer( double freq ) : Timer( freq ) {
}

void TimeoutsTimer::timeout( unsigned nb_expires ) {
    while( nb_expires-- ) {
        ev_loop->timeout_list.shift( [&]( Event *ev ) {
            ev->on_timeout();
        } );
    }
}

bool TimeoutsTimer::need_wait() const {
    return false;
}

} // namespace Evel
