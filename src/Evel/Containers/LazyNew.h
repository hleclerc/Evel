#pragma once

namespace Evel {

///
template<class T>
class LazyNew {
public:
    LazyNew() : val( 0 ) {
    }

    ~LazyNew() {
        delete val;
    }

    T *ptr() {
        if ( not val )
            val = new T;
        return val;
    }

    T *operator->() {
        return ptr();
    }

    T &operator*() {
        return *ptr();
    }

    void clear() {
        delete val;
        val = 0;
    }

private:
    T *val;
};

} // namespace Evel
