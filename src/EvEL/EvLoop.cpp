#include "TimeoutsTimer.h"
#include "PostFunc.h"
#include "WaitOut.h"

#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
 

namespace Evel {

EvLoop::EvLoop() {
    // init epoll
    event_fd = epoll_create1( 0 );
    if ( event_fd < 0 )
        perror( "epoll_create1" );

    // default values
    nb_waiting_events = 0;
    timeouts_timer    = 0;
    wait_out_timer    = 0;
    ret               = 0;
}

EvLoop::~EvLoop() {
    if ( event_fd >= 0 )
        close( event_fd );
    delete wait_out_timer;
    delete timeouts_timer;
}

int EvLoop::run() {
    if ( event_fd < 0 )
        return -1;

    cnt = true;

    // loop
    const int max_events = 1024;
    epoll_event events[ max_events ];
    do {
        // "work" to do
        while( Event *ev = work_list.pop_front() )
            ev->work();

        // (actual) deletions
        while( Event *ev = del_list.pop_front() ) {
            delete ev;
        }

        // if no active event => stop the loop
        if ( nb_waiting_events == 0 )
            break;
        
        // get events and say that they are going to be handled (with protection of epoll_mutex)
        int nfds = epoll_wait( event_fd, events, max_events, -1 );
        if ( nfds == -1 ) {
            if ( errno != EINTR ) {
                err( "epoll_wait", strerror( errno ) );
                return -1;
            }
            continue;
        }

        // handle event types
        for( int n = 0; n < nfds; ++n ) {
            Event *ev = reinterpret_cast<Event *>( events[ n ].data.ptr );

            // want_del => nothing to do
            if ( ev->next_del )
                continue;

            if ( events[ n ].events & EPOLLOUT ) { // ready for output (after a EAGAIN or not)
                ev->on_out();
                if ( ev->want_close_fd && ev->out_are_sent() )
                    ev->del();
            }

            if ( events[ n ].events & EPOLLIN && ev->want_close_fd == false && ev->next_del == 0 ) // we have input data
                ev->on_inp();

            if ( events[ n ].events & EPOLLRDHUP ) // end of the connection (gracefully closed by peer)
                ev->on_rd_hup();

            if ( events[ n ].events & EPOLLHUP ) // end of the connection
                ev->on_hup();

            if ( events[ n ].events & EPOLLERR ) // generic error
                ev->error();
        }
    } while ( cnt );

    return ret;
}

bool EvLoop::stop( int ret_val ) {
    ret = ret_val;
    cnt = false;
    return false;
}

EvLoop &EvLoop::operator<<( Event *ev ) {
    if ( ev->ev_loop || event_fd < 0 )
        return *this;
    ev->ev_loop = this;

    if ( ev->need_wait )
        ++nb_waiting_events;

    epoll_event ee;
    ee.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET; // ET -> trigger
    ee.data.u64 = 0; // for valgrind on 32 bits machines
    ee.data.ptr = ev;
    if ( epoll_ctl( event_fd, EPOLL_CTL_ADD, ev->fd, &ee ) == -1 )
        err( "epoll_ctl add: ", strerror( errno ) );

    ev->on_install();
    ev->__on_rdy();
    return *this;
}

EvLoop &EvLoop::operator>>( Event *ev ) {
    if ( event_fd >= 0 ) {
        if ( epoll_ctl( event_fd, EPOLL_CTL_DEL, ev->fd, 0 ) == -1 )
            err( "epoll_ctl del: ", strerror( errno ) );
        if ( ev->need_wait )
            --nb_waiting_events;
    }
    return *this;
}

void EvLoop::add_work( Event *ev ) {
    work_list.push_back( ev );
}

void EvLoop::add_timeout( TimeoutEvent *ev, double delay ) {
    if ( ! timeouts_timer )
        *this << ( timeouts_timer = new TimeoutsTimer( 0.125 ) );
    timeout_list.add( ev, delay / 0.125 );
    ev->ev_loop = this;
}

void EvLoop::rem_timeout( TimeoutEvent *ev_obj ) {
    timeout_list.rem( ev_obj );
}

void EvLoop::log( const char *msg, const char *cmp ) {
    std::cout << msg << ( cmp ? cmp : "" ) << std::endl;
}

void EvLoop::err( const char *msg, const char *cmp ) {
    std::cerr << msg << ( cmp ? cmp : "" ) << std::endl;
}

void EvLoop::check_wait_out() {
    std::lock_guard<std::mutex> lock( wait_out_mutex );
    if ( ! wait_out_timer ) {
        wait_out_timer = new WaitOut( 0.2 );
        *this << wait_out_timer;
    }
}

} // namespace Evel
