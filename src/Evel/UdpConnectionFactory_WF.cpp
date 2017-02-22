#include "UdpConnectionFactory_WF.h"

namespace Evel {

UdpConnectionFactory_WF::UdpConnectionFactory_WF( TF_factory &&f_factory ) : f_factory( std::move( f_factory ) ) {
}

UdpConnectionFactory_WF::~UdpConnectionFactory_WF() {
    if ( f_on_close ) f_on_close( this );
}

UdpConnection *UdpConnectionFactory_WF::factory( const InetAddress &src ) {
    return f_factory ? f_factory( this, src ) : 0;
}
 
} // namespace Evel
