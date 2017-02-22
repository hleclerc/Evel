#include "../src/Evel/UdpConnectionFactory_WF.h"
#include "../src/Evel/UdpConnection_WF.h"
#include "../src/Evel/UdpSocket_WF.h"
#include "../src/Evel/Timer_WF.h"
#include "../src/Evel/SslCtx.h"
#include "../src/Evel/EvLoop.h"
#include <gtest/gtest.h>
using namespace Evel;

TEST( Udp, raw_socket ) {
    std::string res;
    EvLoop el;

    auto *sock_server = new UdpSocket_WF;
    sock_server->f_parse = [&]( UdpSocket_WF *ucf, const InetAddress &, char **data, unsigned size ) {
        res.append( *data, size );
        ucf->close();
    };
    sock_server->bind( 8748 );
    el << sock_server;

    auto *sock_client = new UdpSocket_WF;
    sock_client->send( { "127.0.0.1", 8748 }, "smurf" );
    sock_client->close();
    el << sock_client;

    auto *timer = new Timer_WF( 2.0, [&]( Timer_WF *, unsigned ) { ADD_FAILURE() << "Timeout"; return el.stop(); }, /* don't block the loop */ false );
    el << timer;

    el.run();

    delete timer;
    EXPECT_EQ( res, "smurf" );
}

TEST( Udp, with_connection ) {
    int nb_closed = 0;
    std::string res;
    EvLoop el;

    auto *sock_server = new UdpConnectionFactory_WF;
    sock_server->f_factory = [ &res, &nb_closed ]( UdpConnectionFactory_WF *ucf, const InetAddress &src ) {
        auto *conn = new UdpConnection_WF( ucf, src );
        conn->f_parse = [ &res, &nb_closed ]( UdpConnection_WF *conn, char **data, size_t size ) {
            res.append( *data, size );
            conn->close();
        };
        conn->f_on_del = [&nb_closed]( UdpConnection_WF *conn ) {
            conn->ucf->close();
            nb_closed += 10;
        };
        return conn;
    };
    sock_server->bind( 8748 );
    el << sock_server;

    auto *sock_client = new UdpConnectionFactory_WF;
    sock_client->f_factory = [ &nb_closed ]( UdpConnectionFactory_WF *ucf, const InetAddress &src ) {
        auto *conn = new UdpConnection_WF( ucf, src );
        conn->f_on_del = [ &nb_closed ]( UdpConnection_WF * ) {
            ++nb_closed;
        };
        return conn;
    };
    auto *conn_client = sock_client->connection( { "127.0.0.1", 8748 } );
    conn_client->send( "smurf" );
    conn_client->close();
    sock_client->close();
    el << sock_client;

    auto *timer = new Timer_WF( 2.0, false );
    timer->f_timeout = [&]( Timer_WF *, unsigned ) {
        ADD_FAILURE() << "Timeout";
        el.stop();
    };
    el << timer;

    el.run();

    delete timer;
    EXPECT_EQ( res, "smurf" );
    EXPECT_EQ( nb_closed, 11 );
}

