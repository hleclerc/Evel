#include "../src/Evel/Signal_WF.h"
#include "../src/Evel/Timer_WF.h"
#include "../src/Evel/EvLoop.h"
#include <gtest/gtest.h>
using namespace Evel;

TEST( Signal, signal ) {
    int c_sig = 0;
    EvLoop el;

    int sigs[] = { SIGINT, SIGQUIT, SIGKILL, SIGUSR1, -1 };
    Signal *sgn = new Signal_WF( sigs, [&c_sig]( Signal_WF *s, int sig ){ c_sig = sig; s->close(); } );
    el << sgn;

    std::string cmd = "sleep 1; kill " + std::to_string( getpid() );
    system( cmd.c_str() );

    el.run();

    EXPECT_EQ( c_sig, 10 );
}

