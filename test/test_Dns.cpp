#include "../src/EvEL/Timer_WF.h"
#include "../src/EvEL/EvLoop.h"
#include "../src/EvEL/Dns.h"
#include "../src/EvEL/Gev.h"
#include <gtest/gtest.h>
using namespace Evel;

TEST( Dns, req ) {
    Dns dns;
    dns.query( "www.google.com", []( int err, const std::vector<InetAddress> &addr ) {
        P( addr );
        gev->stop();
    } );

    *gev << new Timer_WF( 1.0, []( Timer_WF *t, unsigned ) { I( "timer" ); t->close(); }, true );
    gev->run();
    // EXPECT_EQ( c_sig, 10 );
}

