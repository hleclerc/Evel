#include "FileDescriptor.h"
#include "EvLoop.h"
#include "Signal.h"

#include <sys/signalfd.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

namespace Evel {

static int make_signal_fd( const int *sigs ) {
    // set up a mask
    sigset_t mask;
    sigemptyset( &mask );
    for( int i = 0; sigs[ i ] >= 0; ++i )
        sigaddset( &mask, sigs[ i ] );

    // block signals
    if ( sigprocmask( SIG_BLOCK, &mask, 0 ) == -1 ) {
        perror( "sigprocmask" );
        return -1;
    }

    // get a (non blocking) file descriptor
    int fd = signalfd( -1, &mask, 0 );
    if ( fd < 0 )
        perror( "signalfd" );
    if ( set_non_blocking( fd ) < 0 ) {
        perror( "non blocking signalfd" );
        close( fd );
        return -1;
    }

    return fd;
}


Signal::Signal( const int *sigs ) : Event( make_signal_fd( sigs ) ) {
}

void Signal::on_inp() {
    signalfd_siginfo sig_info;
    while ( true ) {
        ssize_t s = read( fd, &sig_info, sizeof( sig_info ) );

        // end of the connection
        if ( s == 0 )
            return close();

        // error
        if ( s < 0 ) {
            if ( errno == EINTR )
                continue;
            if ( errno == EAGAIN or errno == EWOULDBLOCK )
                return;
            ev_loop->err( "Pb reading signals: {}", strerror( errno ) );
            close();
            return;
        }

        if ( s < (ssize_t)sizeof( sig_info ) ) {
            ev_loop->err( "TODO: partial read with signals" );
            close();
            return;
        }

        signal( sig_info.ssi_signo );
    }
}

bool Signal::out_are_sent() const {
    return true;
}

bool Signal::need_wait() const {
    return false;
}

bool Signal::may_have_out() const {
    return false;
}

} // namespace Evel
