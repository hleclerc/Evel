#include "Timer_WF.h"

namespace Evel {

Timer_WF::Timer_WF( double delay, double freq, TF_timeout &&f_timeout, bool need_ev_loop_to_wait ): Timer( delay, freq ), f_timeout( std::move( f_timeout ) ), _need_ev_loop_to_wait( need_ev_loop_to_wait ) {
}

Timer_WF::Timer_WF( double freq, TF_timeout &&f_timeout, bool need_ev_loop_to_wait ): Timer_WF( freq, freq, std::move( f_timeout ), need_ev_loop_to_wait ) {
}

Timer_WF::Timer_WF( double delay, double freq, bool need_wait ): Timer_WF( delay, freq, {}, need_wait ) {
}

Timer_WF::Timer_WF( double freq, bool need_wait ): Timer_WF( freq, freq, {}, need_wait ) {
}

bool Timer_WF::need_wait() const {
    return _need_ev_loop_to_wait;
}

void Timer_WF::timeout( unsigned nb_expirations ) {
    if ( f_timeout )
        f_timeout( this, nb_expirations );
}

} // namespace Evel
