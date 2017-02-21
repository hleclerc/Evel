#pragma once

#include "TypeConfig.h"

namespace Evel {


template<unsigned s>
struct BitHash {
    static PI64 bit_hash( const char *val ) {
        return *reinterpret_cast<const PI64 *>( val ) ^ BitHash< s - 8 >::bit_hash( val + 8 );
    }
};

template<> struct BitHash<0> { static PI64 bit_hash( const char *val ) { return 0; } };
template<> struct BitHash<1> { static PI64 bit_hash( const char *val ) { return *reinterpret_cast<const PI8  *>( val ); } };
template<> struct BitHash<2> { static PI64 bit_hash( const char *val ) { return *reinterpret_cast<const PI16 *>( val ); } };
template<> struct BitHash<3> { static PI64 bit_hash( const char *val ) { return *reinterpret_cast<const PI16 *>( val ) ^ *reinterpret_cast<const PI8  *>( val + 2 ); } };
template<> struct BitHash<4> { static PI64 bit_hash( const char *val ) { return *reinterpret_cast<const PI32 *>( val ); } };
template<> struct BitHash<5> { static PI64 bit_hash( const char *val ) { return *reinterpret_cast<const PI32 *>( val ) ^ *reinterpret_cast<const PI8  *>( val + 4 ); } };
template<> struct BitHash<6> { static PI64 bit_hash( const char *val ) { return *reinterpret_cast<const PI32 *>( val ) ^ *reinterpret_cast<const PI16 *>( val + 4 ); } };
template<> struct BitHash<7> { static PI64 bit_hash( const char *val ) { return *reinterpret_cast<const PI32 *>( val ) ^ *reinterpret_cast<const PI16 *>( val + 4 ) ^ *reinterpret_cast<const PI8 *>( val + 6 ); } };
template<> struct BitHash<8> { static PI64 bit_hash( const char *val ) { return *reinterpret_cast<const PI64 *>( val ); } };

/// simple hash using bits of val
template<class T>
PI64 bit_hash( const T &val ) {
    return BitHash<sizeof(T)>::bit_hash( reinterpret_cast<const char *>( &val ) );
}

}
