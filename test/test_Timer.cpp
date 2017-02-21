#include "../src/EvEL/Timer_WF.h"
#include "../src/EvEL/EvLoop.h"
#include <gtest/gtest.h>
using namespace Evel;

//TEST( Timer, timer ) {
//    EvLoop el;

//    int cpt = 0, inc = 1;
//    el << new Timer_WF( 0.25, 0.5, [&]( Timer_WF *t, unsigned ) { cpt += inc; std::cout << "timer" << std::endl; if ( ! inc ) t->close(); } );
//    el << new Timer_WF( 2.5, [&]( Timer_WF *t, unsigned ) { inc = 0; t->close(); } );

//    el.run();

//    EXPECT_EQ( cpt, 5 );
//}

struct MyTimeoutEvent : public Event {
    virtual void on_timeout() override {
        ev_loop->add_timeout( this, 1.0 );
        I( "timeout" );
        ++cpt;
    }
    virtual bool need_wait() const override {
        return false;
    }
    virtual bool may_have_out() const override {
        return false;
    }
    int cpt = 0;
};

TEST( Timer, timeout ) {
    MyTimeoutEvent mte;
    EvLoop el;

    el << new Timer_WF( 3.0, [&]( Timer_WF *t, unsigned ) { t->close(); } );
    el.add_timeout( &mte, 0.5 );

    el.run();

    EXPECT_EQ( mte.cpt, 3 );
}
