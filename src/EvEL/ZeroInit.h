#pragma once

namespace Evel {

/**
*/
template<class T>
class ZeroInit {
public:
    ZeroInit( const T &val = 0 ) : val( val ) {}

    operator T&() { return val; }

    T val;
};

} // namespace Evel
