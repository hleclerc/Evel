#include "../src/Evel/System/Print.h"
#include "../src/Evel/TcpConnection_WF.h"
#include "../src/Evel/Listener_WF.h"
#include "../src/Evel/Timer_WF.h"
#include "../src/Evel/EvLoop.h"
#include <gtest/gtest.h>
using namespace Evel;

//TEST( Tcp, client ) {
//    size_t msg_len = 0;
//    EvLoop el;

//    auto *conn_client = new TcpConnection_WF( { "127.0.0.1", 8080 } );
//    conn_client->f_parse = [&]( TcpConnection_WF *c, char **data, size_t size, size_t rese ) {
//        conn_client->close(); // should be closed also by the peer
//        msg_len += size;
//    };
//    conn_client->send( "GET /rcds HTTP/1.0\n\n" );
//    el << conn_client;

//    auto *timer = new Timer_WF( 2.0, [&]( Timer_WF *, unsigned ) { ADD_FAILURE() << "Timeout"; return el.stop(); }, /* don't block the loop */ false );
//    el << timer;

//    el.run();

//    delete timer;
//    DISP_INFO( "received from server: {}", msg_len );
//    EXPECT_TRUE( msg_len > 10 );
//}

TEST( Tcp, server_and_client ) {
    size_t msg_len = 0;
    EvLoop el;

    // echo server
    auto *listener = new Listener_WF( 5424, /* block_the_loop */ false );
    listener->f_connection = []( Listener_WF *l, int fd, const InetAddress &addr ) {
        auto *conn_server = new TcpConnection_WF( fd );
        conn_server->f_parse = []( TcpConnection_WF *c, char **data, size_t size, size_t rese ) {
            // I( "sending", *data, size );
            c->send( *data, size );
            c->close();
        };
        *l->ev_loop << conn_server;
    };
    el << listener;

    // client
    auto *conn_client = new TcpConnection_WF( { "127.0.0.1", 5424 } );
    conn_client->f_parse = [&]( TcpConnection_WF *c, char **data, size_t size, size_t rese ) {
        msg_len += size;
        c->close();
    };
    //    conn_client->f_close = []( TcpConnection_WF *c ) {
    //        I( "closing client" )
    //    };
    conn_client->send( "pouet" );
    el << conn_client;

    // a timer for the test
    auto *timer = new Timer_WF( 2.0, [&]( Timer_WF *, unsigned ) { ADD_FAILURE() << "Timeout"; return el.stop(); }, /* block_the_loop */ false );
    el << timer;

    el.run();

    delete timer;
    delete listener;
    EXPECT_TRUE( msg_len == 5 );
}

