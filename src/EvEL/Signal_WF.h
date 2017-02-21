#pragma once

#include <functional>
#include "Signal.h"

namespace Evel {

/**
*/
class Signal_WF : public Signal {
public:
    using TF_signal = std::function<void( Signal_WF *, int sig )>;

    Signal_WF( const int *sigs, TF_signal &&f = {} );

    TF_signal f_signal;

protected:
    virtual void signal( int s );
};

} // namespace Evel
