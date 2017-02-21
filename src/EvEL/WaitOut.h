#pragma once

#include "Timer.h"

namespace Evel {

/**
*/
class WaitOut : public Timer {
public:
    WaitOut( double freq );

    virtual void timeout( unsigned ) override;
};

} // namespace Evel
