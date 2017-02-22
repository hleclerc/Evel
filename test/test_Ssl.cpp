#include "../src/Evel/TlsConnection_WF.h"
#include "../src/Evel/Listener_WF.h"
#include "../src/Evel/Timer_WF.h"
#include "../src/Evel/SslCtx.h"
#include "../src/Evel/EvLoop.h"
#include <gtest/gtest.h>
using namespace Evel;

//TEST( Ssl, client ) {
//    size_t msg_len = 0;
//    EvLoop el;

//    SslCtx ssl_ctx_client{ SslCtx::Method::SSLv23, false };
//    auto *conn_client = new SslConnection_WF( ssl_ctx_client, { "127.0.0.1", 8080 } );
//    //auto *conn_client = new SslConnection_WF( ssl_ctx_client, { "2400:cb00:2048:1::6814:2c07", 443 } ); // random.org
//    conn_client->f_parse = [&]( SslConnection_WF *c, char **data, size_t size, size_t rese ) {
//        // std::cout.write( *data, size );
//        conn_client->close();
//        msg_len += size;
//    };
//    conn_client->send( "GET /rfepoj HTTP/1.0\n\n" );
//    el << conn_client;

//    auto *timer = new Timer_WF( 2.0, [&]( Timer_WF *, unsigned ) { ADD_FAILURE() << "Timeout"; return el.stop(); }, /* don't block the loop */ false );
//    el << timer;

//    el.run();

//    delete timer;
//    EXPECT_TRUE( msg_len > 10 );
//}

TEST( Ssl, server_and_client ) {
    std::string msg;
    EvLoop el;

    SslCtx ssl_ctx_server{ SslCtx::Method::SSLv23, "test/cert.pem", "test/key.pem" };
    SslCtx ssl_ctx_client{ SslCtx::Method::SSLv23, false };

    // echo server
    auto *listener = new Listener_WF( 5424, /* block_the_loop */ false );
    listener->f_connection = [ &ssl_ctx_server ]( Listener_WF *l, int fd, const InetAddress &addr ) {
        auto *conn_server = new TlsConnection_WF( ssl_ctx_server, fd );
        conn_server->f_parse = []( TlsConnection_WF *c, char **data, size_t size, size_t rese ) {
            c->send( *data, size );
            c->close();
        };
        //        conn_server->f_close = [&]( SslConnection_WF *c ) {
        //            I( "close server" );
        //        };
        *l->ev_loop << conn_server;
    };
    el << listener;


    // client
    // system( "curl --insecure https://localhost:5424 &" );
    auto *conn_client = new TlsConnection_WF( ssl_ctx_client, { "127.0.0.1", 5424 } );
    conn_client->f_parse = [&]( TlsConnection_WF *c, char **data, size_t size, size_t rese ) {
        msg.append( *data, size );
        c->close();
    };
    //    conn_client->f_close = [&]( SslConnection_WF *c ) {
    //        I( "close client" );
    //    };
    conn_client->send( "rfepoj" );
    el << conn_client;

    // a timer for the test
    auto *timer = new Timer_WF( 2.0, [&]( Timer_WF *, unsigned ) { ADD_FAILURE() << "Timeout"; return el.stop(); }, /* block_the_loop */ false );
    el << timer;

    el.run();

    delete timer;
    delete listener;
    EXPECT_EQ( msg, "rfepoj" );
}

