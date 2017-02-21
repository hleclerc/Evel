#pragma once

#include "Event.h"

namespace Evel {

/**
*/
class Signal : public Event {
public:
    Signal( const int *sigs, bool need_wait = false );

protected:
    virtual void signal      ( int s ) = 0; ///< has to return true to continue watching for signal. To stop the program, one can use ev_loop->stop()

private:
    virtual bool may_have_out() const override;
    virtual void on_inp         () override;
    virtual bool out_are_sent() const override;
};

} // namespace Evel
