#include "TimeoutEvent.h"

namespace Evel {

TimeoutEvent::TimeoutEvent() {
    ev_loop = 0;
}

TimeoutEvent::TimeoutEvent( VtableOnly ) {
}

TimeoutEvent::~TimeoutEvent() {
}

} // namespace Evel
