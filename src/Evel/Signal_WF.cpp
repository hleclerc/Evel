#include "Signal_WF.h"

namespace Evel {

Signal_WF::Signal_WF( const int *sigs, TF_signal &&f_signal ): Signal( sigs ), f_signal( std::move( f_signal ) ) {
}

void Signal_WF::signal( int s ) {
    if ( f_signal ) f_signal( this, s );
}

} // namespace Evel
