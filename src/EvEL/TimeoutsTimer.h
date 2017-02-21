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
};

} // namespace Evel
