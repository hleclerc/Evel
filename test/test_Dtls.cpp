#include "../src/EvEL/UdpConnectionFactory_WF.h"
#include "../src/EvEL/UdpConnectionDTLS_WF.h"
#include "../src/EvEL/Timer_WF.h"
#include "../src/EvEL/SslCtx.h"
#include "../src/EvEL/EvLoop.h"
#include <gtest/gtest.h>
using namespace Evel;

TEST( Udp, DTLS_rdy_send ) {
    SslCtx ssl_ctx_server( SslCtx::Method::DTLSv1, "test/cert.pem", "test/key.pem" );
    SslCtx ssl_ctx_client( SslCtx::Method::DTLSv1, false );
    int nb_closed = 0;
    std::string res;
    EvLoop el;

    // instantiation and registering of the server socket
    auto *sock_server = new UdpConnectionFactory_WF;
    sock_server->f_factory = [&]( UdpConnectionFactory *ucf, const InetAddress &src ) {
        auto *conn = new UdpConnectionDTLS_WF( ssl_ctx_server, true, ucf, src );
        I( "create conn server" );
        conn->f_parse = [&]( UdpConnectionDTLS_WF *conn, char **data, ssize_t size ) {
            res.append( *data, size );
            conn->close();
        };
        conn->f_on_close = [&]( UdpConnectionDTLS_WF *conn ) {
            I( "close conn server" );
            conn->ucf->close();
            nb_closed += 10;
        };
        return conn;
    };
    sock_server->bind( 8749 );
    el << sock_server;

    // creation of DTLS "rdy" client connections
    auto fact_client = [&]( UdpConnectionFactory *ucf, const InetAddress &src ) {
        UdpConnectionDTLS_WF *conn = new UdpConnectionDTLS_WF( ssl_ctx_client, false, ucf, src );
        conn->f_on_rdy = []( UdpConnectionDTLS_WF *conn ) {
            conn->send( "smurf" );
            conn->close();
        };
        conn->f_on_close = [&]( UdpConnectionDTLS_WF *conn ) {
            I( "close conn client" );
            conn->ucf->close();
            ++nb_closed;
        };
        return conn;
    };

    // first client, using rdy()
    auto *sock_client = new UdpConnectionFactory_WF( fact_client );
    sock_client->connection( { "127.0.0.1", 8749 } );
    sock_client->f_on_close = []( UdpConnectionFactory_WF * ) {
        I( "close sock client" );
    };
    el << sock_client;

    // check (false => do no block the loop)
    auto *timer = new Timer_WF( 2.0, false );
    timer->f_timeout = [&]( Timer_WF *, unsigned ) {
        ADD_FAILURE() << "Timeout"; return el.stop();
    };
    el << timer;

    el.run();

    delete timer;
    EXPECT_EQ( nb_closed, 11 );
    EXPECT_EQ( res, "smurf" );
}

// TEST( Udp, DTLS_imm_send ) {
//     SslCtx ssl_ctx_server( SslCtx::Method::DTLSv1, "test/cert.pem", "test/key.pem" );
//     SslCtx ssl_ctx_client( SslCtx::Method::DTLSv1 );
//     int nb_closed = 0;
//     std::string res;
//     EvLoop el;

//     // instantiation and registering of the server socket
//     auto *sock_server = new UdpConnectionFactory_WF;
//     sock_server->f_factory = [&res,&nb_closed,&ssl_ctx_server]( UdpConnectionFactory *ucf, const InetAddress &src ) {
//         auto *conn = new UdpConnectionDTLS_WF( ssl_ctx_server, true, ucf, src );
//         conn->f_parse = [&res]( UdpConnectionDTLS_WF *conn, char **data, ssize_t size ) {
//             res.append( *data, size );
//             conn->close();
//         };
//         conn->f_on_close = [&nb_closed]( UdpConnectionDTLS_WF *conn ) {
//             conn->ucf->close();
//             nb_closed += 10;
//         };
//         return conn;
//     };
//     sock_server->bind( 8749 );
//     el << sock_server;

//     // creation of basic DTLS client connections
//     auto fact_client = [&nb_closed,&ssl_ctx_client]( UdpConnectionFactory *ucf, const InetAddress &src ) {
//         UdpConnectionDTLS_WF *conn = new UdpConnectionDTLS_WF( ssl_ctx_client, false, ucf, src );
//         conn->f_on_close = [&nb_closed]( UdpConnectionDTLS_WF *conn ) {
//             ++nb_closed;
//         };
//         return conn;
//     };

//     // second client, using send directly (=> use of a buffer)
//     auto *sock_client = new UdpConnectionFactory_WF( fact_client );
//     sock_client->send( { "127.0.0.1", 8749 }, "smurf" );
//     sock_client->close(); // => check inp and handshake before del
//     el << sock_client;

//     // check (false => do no block the loop)
//     auto *timer = new Timer_WF( 2.0, false );
//     timer->f_timeout = [&]( Timer_WF *, unsigned ) {
//         ADD_FAILURE() << "Timeout"; return el.stop();
//         el.stop();
//     };
//     el << timer;

//     el.run();

//     delete timer;
//     EXPECT_EQ( nb_closed, 11 );
//     EXPECT_EQ( res, "smurf" );
// }

