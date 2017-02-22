#pragma once

#include "../Containers/TypeConfig.h"
#include "../Containers/RcPtr.h"

#include <stdlib.h>

namespace Evel {

/**
*/
class SharedBuffer {
public:
    enum { default_size = 2048 - 3 * sizeof( unsigned ) - sizeof( SharedBuffer * ) };

    static SharedBuffer *New( unsigned size = default_size, SharedBuffer *prev = 0 ) {
        SharedBuffer *res = (SharedBuffer *)malloc( sizeof( SharedBuffer ) + size - 4 );
        if ( prev ) prev->next = res;
        res->cpt_use = 0;
        res->used    = 0;
        res->next    = 0;
        res->size    = size;
        return res;
    }

    static void operator delete( void *ptr ) {
        free( ptr );
    }

    unsigned room() const {
        return size - used;
    }

    PT cum_size() const {
       PT res = 0;
       for( const SharedBuffer *b = this; b; b = b->next )
           res += b->used;
       return res;
    }

    // attributes
    mutable int   cpt_use;   ///< destroyed if < 0
    unsigned      used;      ///< nb items stored in data
    SharedBuffer *next;      ///<
    unsigned      size;      ///< real size of data[]
    PI8           data[ 4 ]; ///<
};

}
