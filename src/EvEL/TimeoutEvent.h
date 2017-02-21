#pragma once

#include "ExpIndexedList.h"

namespace Evel {

class TimeoutsTimer;
class EvLoop;

/**
*/
class TimeoutEvent {
public:
    struct VtableOnly {};

    TimeoutEvent();                    ///< fd = file descriptor
    TimeoutEvent( VtableOnly );        ///< a constructor that does not assign any attribute (else than the vtable). Allows to do a new( ptr ) T to change _only_ the vtable (underlying type).
    virtual ~TimeoutEvent();

    EvLoop          *ev_loop;          ///< the event_loop where `this` is registered

protected:
    struct           GetTimeoutData { template<class T> auto &operator()( T *item ) const { return item->timeout_data; } };
    using            TimeoutData = ExpIndexedItemData<TimeoutEvent,GetTimeoutData>;
    friend class     TimeoutsTimer;
    friend class     EvLoop;

    virtual void     on_timeout() = 0; ///<

    TimeoutData      timeout_data;
};

} // namespace Evel
