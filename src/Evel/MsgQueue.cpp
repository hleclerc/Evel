#include "MsgQueue.h"
#include <string.h>

namespace Evel {

MsgQueue::Msg *MsgQueue::Msg::New( unsigned size ) {
    Msg *res = (Msg *)malloc( sizeof( Msg ) + size - 4 );
    res->next = nullptr;
    res->size = 0;
    return res;
}

void MsgQueue::Msg::write_to_stream( std::ostream &os ) const {
    int cpt = 0;
    static const char *c = "0123456789abcdef";
    for( PT i = 0; i < size; ++i )
        os << ( cpt++ ? " " : "" ) << c[ data[ i ] / 16 ] << c[ data[ i ] % 16 ];
}

MsgQueue::MsgQueue() {
    first   = nullptr;
    last    = nullptr;
}

MsgQueue::~MsgQueue() {
    for( Msg *msg = first, *nxt; msg; msg = nxt ) {
        nxt = msg->next;
        free( msg );
    }
}

void MsgQueue::write_to_stream( std::ostream &os ) const {
    for( Msg *msg = first; msg; msg = msg->next ) {
        if ( msg != first ) os << " ";
        os << "[" << *msg << "]";
    }
    if ( first ) os << " ";
}

void MsgQueue::operator<<( Msg *msg ) {
    if ( last ) {
        last->next = msg;
        last = msg;
    } else {
        first = msg;
        last = msg;
    }
}

} // namespace Evel
