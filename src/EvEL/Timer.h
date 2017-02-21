#pragma once

#include "Event.h"
#include <functional>

namespace Evel {

/**
*/
class Timer : public Event {
public:
    Timer( double delay, double freq ); ///<
    Timer( double freq ); ///<

protected:
    virtual void timeout      ( unsigned nb_expirations ) = 0; ///< has return true to keep the timer running

private:
    static int   make_timer_fd( double delay, double freq );
    virtual bool may_have_out () const override;
    virtual void on_inp       () override;
    virtual bool out_are_sent () const override;
};

} // namespace Evel
