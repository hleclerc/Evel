#pragma once

#include "../Containers/TypeConfig.h"
#include "../Containers/RcPtr.h"

#include <stdlib.h>
#include <new>

namespace Evel {

/**
*/
class SharedBuffer {
public:
    enum { default_size = 2048 - 3 * sizeof( unsigned ) - sizeof( SharedBuffer * ), nb = 4 };

    SharedBuffer( unsigned used, unsigned rese, SharedBuffer *prev = 0 ) : cpt_use( 0 ), used( used ), next( 0 ), rese( rese ) {
        if ( prev )
            prev->next = this;
    }

    static SharedBuffer *New( unsigned size = default_size, SharedBuffer *prev = 0 ) {
        return new ( malloc( sizeof( SharedBuffer ) - nb + size ) ) SharedBuffer( 0, size, prev );
    }

    static void operator delete( void *ptr ) {
        free( ptr );
    }

    unsigned room() const {
        return rese - used;
    }

    PT cum_size() const {
       PT res = 0;
       for( const SharedBuffer *b = this; b; b = b->next )
           res += b->used;
       return res;
    }

    const PI8 *begin() const {
        return data;
    }

    PI8 *begin() {
        return data;
    }

    const PI8 *end() const {
        return data + used;
    }

    PI8 *end() {
        return data + used;
    }

    // attributes
    mutable int   cpt_use;    ///< destroyed if < 0
    unsigned      used;       ///< nb items stored in data
    SharedBuffer *next;       ///<
    unsigned      rese;       ///< real size of data[]
    PI8           data[ nb ]; ///<
};

}
