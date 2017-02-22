#pragma once

#include "InetAddress.h"
#include "Event.h"
#include <vector>
#include <deque>

namespace Evel {

/**
*/
class Listener : public Event {
public:
    Listener( unsigned port, bool need_wait = true ); ///<
    Listener( VtableOnly );

protected:
    virtual void connection  ( int fd, const InetAddress &addr ) = 0; ///< called after an accept

    virtual void on_inp      () override;
    virtual bool out_are_sent() const override;
};

} // namespace Evel
