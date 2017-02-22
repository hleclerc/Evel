#include "CbStringPtr.h"
#include <string.h>

namespace Evel {

CbStringPtr::CbStringPtr( const CbStringPtr &bp ) : beg( bp.beg ), off( bp.off ), end( bp.end ) {
}

CbStringPtr::CbStringPtr( const CbString &bs ) {
    beg = bs.beg;
    off = bs.off;
    end = bs.end;
}

CbStringPtr::CbStringPtr( const CbQueue &bs ) {
    beg = bs.beg;
    off = bs.off;
    end = bs.off + bs.size();
}

bool CbStringPtr::ack_error() {
    off = -1;
    end = -1;
    return false;
}

void CbStringPtr::read_some( void *data, PT size ) {
    if ( not size )
        return;
    visitor( [ this, &data, &size ]( const SharedBuffer *b, PT bd, PT ed ) {
        // last buffer to be used for this reading ?
        PT lm = ed - bd;
        if ( size <= lm ) {
            memcpy( data, b->data + bd, size );
            off += size;
            return false;
        }

        // else, read what we can
        size -= lm;
        memcpy( data, b->data + bd, lm );
        data = reinterpret_cast<PI8 *>( data ) + lm;

        // and remove b from the stream
        end -= b->used;
        beg = b->next;
        off = 0;
        return true;
    } );
}

void CbStringPtr::skip_some( PT size ) {
    visitor( [ this, &size ]( const SharedBuffer *b, PT bd, PT ed ) {
        // last buffer to be used for this reading ?
        PT lm = ed - bd;
        if ( size <= lm ) {
            off += size;
            return false;
        }

        // else, read what we can
        size -= lm;

        // and remove b from the stream
        end -= b->used;
        beg = b->next;
        off = 0;
        return true;
    } );
}

void CbStringPtr::write_to_stream( std::ostream &os ) const {
    visitor( [ &os ]( const SharedBuffer *b, PT beg, PT end ) {
        static const char *c = "0123456789abcdef";
        for( PT i = beg; i < end; ++i )
            os << c[ b->data[ i ] / 16 ] << c[ b->data[ i ] % 16 ] << " ";
        return true;
    } );
}

Evel::CbStringPtr::operator std::string() const {
    std::string res; res.reserve( size() );

    data_visitor( [ &res ]( const PI8 *b, const PI8 *e ) {
        res.append( b, e );
    } );

    return res;
}



} // namespace Evel

