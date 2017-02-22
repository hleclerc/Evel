#include "UdpConnection_WF.h"

namespace Evel {

UdpConnection_WF::UdpConnection_WF( UdpConnectionFactory *ucf, const InetAddress &addr, TF_parse &&f_parse ) : UdpConnection( ucf, addr ), f_parse( std::move( f_parse ) ) {
}

UdpConnection_WF::~UdpConnection_WF() {
    if ( f_on_del ) f_on_del( this );
}

void UdpConnection_WF::parse( char **data, size_t size ) {
    if ( f_parse ) f_parse( this, data, size );
}

} // namespace Evel
