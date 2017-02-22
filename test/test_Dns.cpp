#include "../src/EvEL/Timer_WF.h"
#include "../src/EvEL/Dns_WF.h"
#include "../src/EvEL/EvLoop.h"
#include <gtest/gtest.h>
using namespace Evel;

TEST( Dns, req ) {
    EvLoop el;

    Dns *dns = new Signal_WF( sigs, [&c_sig]( Signal_WF *s, int sig ){ c_sig = sig; s->close(); } );
    el << sgn;

    std::string cmd = "sleep 1; kill " + std::to_string( getpid() );
    system( cmd.c_str() );

    el.run();

    EXPECT_EQ( c_sig, 10 );
}

