#include "WaitOut.h"
#include "EvLoop.h"
#include "Print.h"
#include "Event.h"

#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

namespace Evel {

Event::Event( int fd, bool need_wait ) : fd( fd ), need_wait( need_wait ) {
    next_wait_out   = 0;
    next_work       = 0;
    next_del        = 0;
    del_on_install  = 0;
    work_on_install = 0;
    want_close_fd   = 0;
    has_error       = 0;
}

Event::Event( VtableOnly vo ) : TimeoutEvent( vo ) {
}

Event::~Event() {
    if ( fd > 2 && ev_loop )
        *ev_loop >> this;
    if ( fd > 2 )
        ::close( fd );
}

void Event::close() {
    close_fd();
}

void Event::close_fd() {
    if ( has_error )
        return del();

    if ( want_close_fd )
        return;
    want_close_fd = true;

    if ( has_error || out_are_sent() )
        del();
}

void Event::del() {
    // make the deletion later if ev_loop is not yet installed
    if ( ev_loop ) {
        // fd still has data to send ?
        size_t pending = 0;
        if ( fd >= 0 && has_error == false && may_have_out() && ioctl( fd, SIOCOUTQ, &pending ) )
            ev_loop->err( "ioctl SIOCOUTQ (in the main event loop): ", strerror( errno ) );

        // register in the right list
        if ( pending ) {
            ev_loop->wait_out_list.push_back( this );
            ev_loop->check_wait_out(); // to be done after the push back (because WaitLoop may run in another thread)
        } else {
            ev_loop->del_list.push_back( this );
        }
    } else
        del_on_install = true;
}

void Event::reg_work() {
    // make the work later if ev_loop is not yet installed
    if ( ev_loop )
        ev_loop->work_list.push_back( this );
    else
        work_on_install = true;
}

void Event::on_rd_hup() {
    has_error = true;
    close_fd();
}

void Event::on_hup() {
    has_error = true;
    close_fd();
}

void Event::sys_error() {
    has_error = true;
    close_fd();
}

void Event::error() {
    has_error = true;
    close_fd();
}

void Event::on_inp() {
}

void Event::on_out() {
}

void Event::_on_rdy() {
    on_rdy();
}

void Event::__on_rdy() {
    if ( work_on_install )
        work();
    if ( del_on_install )
        del();

    _on_rdy();
}

void Event::on_rdy() {
}

void Event::on_install() {
}

void Event::on_timeout() {
    close_fd();
}

void Event::work() {
}

bool Event::may_have_out() const {
    return true;
}

bool Event::out_are_sent() const {
    return true;
}

} // namespace Evel
