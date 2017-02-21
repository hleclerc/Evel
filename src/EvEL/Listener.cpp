#include "System/SocketUtil.h"
#include "Listener.h"
#include "EvLoop.h"
#include <sys/socket.h>
#include <string.h>

namespace Evel {

Listener::Listener( unsigned port ) {
    // create an ipv6 socket
    while ( true ) {
        fd = socket( AF_INET6, SOCK_NONBLOCK | SOCK_STREAM, 0 );
        if ( fd < 0 ) {
            if ( errno == EINTR )
                continue;
            ev_loop->err( "socket error: ", strerror( errno ) );
            return;
        }
        break;
    }

    // no "Address already in use" error message
    int yes = 1;
    while ( true ) {
        if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( yes ) ) < 0 ) {
            if ( errno == EINTR )
                continue;
            ev_loop->err( "setsockopt( SO_REUSEADDR ) failed: ", strerror( errno ) );
        }
        break;
    }
    while ( true ) {
        if ( setsockopt( fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof( yes ) ) < 0 ) {
            if ( errno == EINTR )
                continue;
            ev_loop->err( "setsockopt( SO_REUSEPORT ) failed: ", strerror( errno ) );
        }
        break;
    }

    // bind
    InetAddress ia( "::", port );
    while ( true ) {
        if ( bind( fd, (const struct sockaddr *)&ia.sa, sizeof ia.sa ) ) {
            if ( errno == EINTR )
                continue;
            ev_loop->err( "bind failed: ", strerror( errno ) );
            return;
        }
        break;
    }

    // listen
    while ( true ) {
        if ( listen( fd, SOMAXCONN ) == -1 ) {
            if ( errno == EINTR )
                continue;
            ev_loop->err( "listen failed: ", strerror( errno ) );
            return;
        }
        break;
    }
}

Listener::Listener( VtableOnly ) {
}

void Listener::on_inp() {
    while ( true ) {
        // accept
        InetAddress ia;
        socklen_t len_ia_sa = sizeof ia.sa;
        int conn_sock = accept( fd, (struct sockaddr *)&ia.sa, &len_ia_sa );
        if ( conn_sock < 0 ) {
            if ( errno == EINTR )
                continue;
            if ( errno == EWOULDBLOCK || errno == EAGAIN )
                return;
            ev_loop->err( "accept failed: ", strerror( errno ) );
            return;
        }

        // non_blocking
        set_non_block( conn_sock );

        // call `connection`
        connection( conn_sock, ia );
    }
}

bool Listener::out_are_sent() const {
    return true;
}

} // namespace Evel
