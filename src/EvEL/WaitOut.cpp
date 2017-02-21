#include <linux/sockios.h>
#include <sys/ioctl.h>
#include "WaitOut.h"
#include "EvLoop.h"
#include <string.h>

namespace Evel {

WaitOut::WaitOut( double freq ) : Timer( freq ) {
}

void WaitOut::timeout( unsigned ) {
    if ( Event *ev = ev_loop->wait_out_list.disconnect() ) {
        CiList<Event,EvLoop::NO> n_wait_out_list;
        while ( true ) {
            Event *nx = ev->next_wait_out;

            size_t pending = 0;
            if ( ev->fd >= 0 && ioctl( ev->fd, SIOCOUTQ, &pending ) )
                ev_loop->err( "ioctl SIOCOUTQ (in the main event loop): ", strerror( errno ) );

            if ( pending ) {
                ev->next_wait_out = 0; // to be really pushed
                n_wait_out_list.push_back( ev );
            } else
                delete ev;

            if ( nx == ev )
                break;
            ev = nx;
        }

        // push the "renewed" events in the front
        ev_loop->wait_out_list.prepend( n_wait_out_list );

        // close if necessary
        std::lock_guard<std::mutex> lock( ev_loop->wait_out_mutex );
        if ( ev_loop->wait_out_list.empty() ) { // we add
            ev_loop->wait_out_timer = 0;
            close();
        }
    }
}

} // namespace Evel
