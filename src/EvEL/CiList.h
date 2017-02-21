#pragma once

#include <mutex>

namespace Evel {

struct GetNext { template<class T> T *&operator()( T *item ) const { return item->next; } };

/** Singly-linked intrusive List with the last element referencing itself
 *  (to have a simple criterion to determine if an element is already in the list or not)
 *  uses mutex for now. TODO: use atomics
*/
template<class T,class GN=GetNext>
class CiList {
public:
    CiList() : head( nullptr ), tail( nullptr ) {}

    /// add a new item
    void push_back( T *item ) {
        std::lock_guard<std::mutex> lock( mutex );
        if ( ! next( item ) ) {
            ( tail ? next( tail ) : head ) = item;
            next( item ) = item;
            tail = item;
        }
    }

    ///
    T *pop_front() {
        std::lock_guard<std::mutex> lock( mutex );
        if ( T *res = head ) {
            if ( next( res ) == res ) {
                head = nullptr;
                tail = nullptr;
            } else {
                head = next( res );
            }
            next( res ) = nullptr;
            return res;
        }
        return nullptr;
    }

    ///
    bool empty() const {
        return ! head;
    }

    /// [ ...a, ...b ] in b
    void prepend( CiList &a ) {
        CiList &b = *this;
        std::lock_guard<std::mutex> lock_a( a.mutex );
        std::lock_guard<std::mutex> lock_b( b.mutex );
        if ( b.head ) {
            if ( a.head ) {
                next( a.tail ) = b.head;
                b.head = a.head;
            }
        } else {
            b.head = a.head;
            b.tail = a.tail;
        }
    }

    ///
    T *disconnect() {
        std::lock_guard<std::mutex> lock( mutex );
        T *res = head;
        head = 0;
        tail = 0;
        return res;
    }

    T         *head;
    T         *tail;
    std::mutex mutex;
    GN         next;
};

} // namespace Evel
