#include "../src/EvEL/Timer_WF.h"
#include "../src/EvEL/EvLoop.h"
#include <gtest/gtest.h>
using namespace Evel;

TEST( Timer, timer ) {
    EvLoop el;

    int cpt = 0, inc = 1;
    el << new Timer_WF( 0.25, 0.5, [&]( Timer_WF *t, unsigned ) { cpt += inc; std::cout << "timer" << std::endl; if ( ! inc ) t->close(); } );
    el << new Timer_WF( 2.5, [&]( Timer_WF *t, unsigned ) { inc = 0; t->close(); } );

    el.run();

    EXPECT_EQ( cpt, 5 );
}
