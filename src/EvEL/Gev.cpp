#include "Gev.h"

namespace Evel {

LazyNew<EvLoop> gev;

//int Gev::run( double timeout ) {
////    if ( timeout )
////        add_timer( [ this ]( int ) { stop(); return false; }, timeout );
//    return EvLoop::run();
//}

//void Gev::add_timer( std::function<bool(int)> &&f, double delay, double freq ) {
//    struct TimerFunc : Timer {
//        TimerFunc( std::function<bool(int)> &&f, double delay, double freq ) : Timer( delay, freq ), f( std::move( f ) ) {
//        }
//        virtual bool timeout( int nb_expirations ) {
//            return f( nb_expirations );
//        }
//        std::function<bool(int)> f;
//    };
//    *this << new TimerFunc{ std::move( f ), delay, freq };
//}

} // namespace Evel
