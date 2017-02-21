#pragma once

#include "Timer.h"

namespace Evel {

/**
*/
class TimeoutsTimer : public Timer {
public:
    TimeoutsTimer( double freq );

protected:
    virtual void timeout( unsigned ) override;
    virtual bool need_wait() const override;
};

} // namespace Evel
