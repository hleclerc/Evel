#pragma once

#include <functional>
#include <stddef.h>
#include "Print.h"

namespace Evel {

/// @see ExpIndexedList
template<class T>
struct ExpIndexedItemData {
    ExpIndexedItemData() : lst( nullptr ) {
    }

    T    **lst;
    T     *prev;
    T     *next;
    size_t offset;
};

/// #see ExpIndexedList
struct GetExpIndexedItemData {
    template<class T>
    ExpIndexedItemData<T> &operator()( T *item ) const {
        return item->exp_indexed_item_data;
    }
    template<class T>
    const ExpIndexedItemData<T> &operator()( const T *item ) const {
        return item->exp_indexed_item_data;
    }
};

/// @see ExpIndexedList
template<class T,class GID=GetExpIndexedItemData,size_t sa=16,size_t nb=4,size_t pm=1,size_t of=0>
struct _ExpIndexedList {
    static constexpr size_t next_index = of + pm * sa;
    using TNext = _ExpIndexedList<T,GID,sa,nb-1,pm*sa,next_index>;

    _ExpIndexedList() {
        for( size_t i = 0; i < sa; ++i )
            sub_lists[ i ] = nullptr;
        cur_list = 0;
    }

    void write_to_stream( std::ostream &os ) const {
        for( size_t i = 0; i < sa; ++i ) {
            // if ( i )
            os << "+";
            int b = 0;
            for( const T *ptr = sub_lists[ ( cur_list + i ) % sa ]; ptr; ptr = gid( ptr ).next )
                os << ( b++ ? " " : "" ) << *ptr;
        }
        os << "|";
        next.write_to_stream( os );
    }

    void add( T *item, size_t index ) {
        index >= next_index ?
            next.add( item, index ) :
            _add_loc( item, index - of );
    }

    void shift( std::function<void(T *item)> f, size_t glob_offset ) {
        // after a cycle, we dispatch items from next->cur_list to *this
        if ( cur_list == sa ) {
            cur_list = 0;
            next.shift( [&]( T *item ) {
                _add_loc( item, gid( item ).offset - glob_offset );
            }, glob_offset );
        }

        // get and remove items from cur list
        T *&lst = sub_lists[ cur_list++ ];
        for( T *ptr = lst; ptr; ) {
            T *item = ptr;
            ptr = gid( ptr ).next;
            gid( item ).lst = 0;
            f( item );
        }
        lst = 0;
   }

    void _add_loc( T *item, size_t index ) {
        T *&lst = sub_lists[ ( cur_list + index / pm ) % sa ];
        if ( lst ) gid( lst ).prev = item;
        gid( item ).prev = nullptr;
        gid( item ).lst  = &lst;
        gid( item ).next = lst;
        lst = item;
    }

    T    *sub_lists[ sa ];
    int   cur_list;
    TNext next;
    GID   gid;
};

// last bucket (for indices from of)
template<class T,class GID,size_t sa,size_t pm,size_t of>
struct _ExpIndexedList<T,GID,sa,0,pm,of> {
    _ExpIndexedList() {
        sub_lists[ 0 ] = nullptr;
    }

    void write_to_stream( std::ostream &os ) const {
        for( T *ptr = sub_lists[ 0 ]; ptr; ptr = gid( ptr ).next )
            os << *ptr << " ";
    }

    void add( T *item, size_t ) {
        T *&lst = sub_lists[ 0 ];
        if ( lst ) gid( lst ).prev = item;
        gid( item ).prev = nullptr;
        gid( item ).lst  = &lst;
        gid( item ).next = lst;
        lst = item;
    }

    void shift( std::function<void(T *item)> f, size_t offset ) {
        T *&lst = sub_lists[ 0 ];

        for( T *ptr = lst; ptr; ) {
            T *item = ptr;
            ptr = gid( ptr ).next;

            if ( gid( item ).offset < offset + pm ) {
                // remove item
                gid( item ).lst = 0;
                if ( gid( item ).prev )
                    gid( gid( item ).prev ).next = gid( item ).next;
                else
                    lst = gid( item ).next;
                if ( gid( item ).next )
                    gid( gid( item ).next ).prev = gid( item ).prev;

                // call f
                f( item );
            }
        }
    }

    T    *sub_lists[ 0 ];
    GID   gid;
};


/**
  intrusive map unsigned => T, with operations
  * add => insert an item at a given index if not already in the list
  * rem => remove an item
  * shift => get and remove T at index = 0 and do --indices (virtually) on each item

  add and rem are O(1)
  shift can be fast (from time to time, it has to re-dispatch items) if sa and nb are high enough to avoid having to much item in a bucket...

  Parameters:
  * T  => stored type (using pointer)
  * GN => get prev and next
  * sa => size of sub-arrays (the first array is used for indices from 0 to sa, the second for indices from sa to sa + sa * sa, ...)
  * nb => nb sub-arrays

  For instance ExpIndexedList<x,y,4,3> will have lists for indices
     0 ->  1 (step = 1)
     1 ->  2
     2 ->  3
     3 ->  4

     4 ->  8 (step = 4)
     8 -> 12
    12 -> 16
    16 -> 20

    20 -> 36 (step = 4*4)
    36 -> 52
    52 -> 68
    68 -> 84

    84 -> .. (remaining items)

*/
template<class T,class GID=GetExpIndexedItemData,size_t sa=16,size_t nb=4>
class ExpIndexedList {
public:
    using List = _ExpIndexedList<T,GID,sa,nb>;

    ExpIndexedList( size_t offset = 0 ) : offset( offset ) {
    }

    void write_to_stream( std::ostream &os ) const {
        list.write_to_stream( os );
    }

    void add( T *item, size_t offset ) {
        if ( gid( item ).lst )
            return;
        list.add( item, offset >= this->offset ? offset - this->offset : 0 );
        gid( item ).offset = offset;
    }

    void rem( T *item ) {
        if ( T **lst = gid( item ).lst ) {
            gid( item ).lst = 0;

            if ( gid( item ).prev )
                gid( gid( item ).prev ).next = gid( item ).next;
            else
                *lst = gid( item ).next;
            if ( gid( item ).next )
                gid( gid( item ).next ).prev = gid( item ).prev;
        }
    }

    void shift( std::function<void(T *item)> f ) {
        list.shift( f, offset++ );
    }

    size_t offset;
    List   list;
    GID    gid;
};

} // namespace Evel

