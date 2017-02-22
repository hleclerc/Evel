#pragma once

namespace Evel {

struct GetPrevNext {
    template<class T> T *&prev( T *v ) const { return v->prev; }
    template<class T> T *&next( T *v ) const { return v->next; }
};

} // namespace Evel
