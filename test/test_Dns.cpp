#include "../src/Evel/System/Print.h"
#include "../src/Evel/Timer_WF.h"
#include "../src/Evel/EvLoop.h"
#include "../src/Evel/Dns.h"
#include "../src/Evel/Gev.h"
#include <gtest/gtest.h>
using namespace Evel;

TEST( Dns, req ) {
    Dns dns;
    dns.timeout = 1.0;

    bool has_addr = false;
    dns.query( "www.google.com", [&]( int err, const std::vector<InetAddress> &addr ) {
        has_addr = addr.size();
        P( err, addr );
    } );

    *gev << new Timer_WF( 2.0, []( Timer_WF *t, unsigned ) { I( "timer" ); t->close(); }, false );
    gev->run();

    EXPECT_EQ( has_addr, true );
}

