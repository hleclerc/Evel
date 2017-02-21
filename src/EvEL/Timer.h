#pragma once

#include "Event.h"
#include <functional>

namespace Evel {

/**
*/
class Timer : public Event {
public:
    Timer( double delay, double freq, bool need_wait = true ); ///<
    Timer( double freq, bool need_wait = true ); ///<

protected:
    virtual void timeout      ( unsigned nb_expirations ) = 0; ///< has return true to keep the timer running

private:
    virtual bool may_have_out () const override;
    virtual void on_inp       () override;
    virtual bool out_are_sent () const override;
};

} // namespace Evel
