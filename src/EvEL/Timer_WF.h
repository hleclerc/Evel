#pragma once

#include "Timer.h"

namespace Evel {

///
class Timer_WF : public Timer {
public:
    using TF_timeout = std::function<void( Timer_WF *, unsigned )>;

    Timer_WF( double delay, double freq, TF_timeout &&f_timeout, bool need_wait = true );
    Timer_WF( double freq, TF_timeout &&f_timeout, bool need_wait = true );
    Timer_WF( double delay, double freq, bool need_wait = true );
    Timer_WF( double freq, bool need_wait = true );

    TF_timeout f_timeout;

protected:
    virtual bool need_wait() const override;
    virtual void timeout  ( unsigned nb_expirations ) override;

    bool _need_ev_loop_to_wait;
};

} // namespace Evel
