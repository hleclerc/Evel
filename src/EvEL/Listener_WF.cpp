#include "Listener_WF.h"

namespace Evel {

Listener_WF::Listener_WF( unsigned port, bool need_wait ) : Listener( port ), _need_wait( need_wait ) {

}

Listener_WF::~Listener_WF() {
    if ( f_close ) f_close( this );
}

void Listener_WF::on_rdy() {
    if ( f_rdy ) f_rdy( this );
}

void Listener_WF::connection( int fd, const InetAddress &addr ) {
    if ( f_connection ) f_connection( this, fd, addr );
}

bool Listener_WF::need_wait() const {
    return _need_wait;
}

} // namespace Evel
