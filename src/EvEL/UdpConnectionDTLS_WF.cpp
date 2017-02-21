#include "UdpConnectionFactory.h"
#include "UdpConnectionDTLS_WF.h"

namespace Evel {

UdpConnectionDTLS_WF::UdpConnectionDTLS_WF( SSL_CTX *ctx, bool server, UdpConnectionFactory *ucf, const InetAddress &addr, TF_parse &&f_parse, TF_on_rdy &&f_rdy, TF_on_close &&f_close ) : UdpConnectionDTLS( ctx, server, ucf, addr ), f_parse( std::move( f_parse ) ), f_on_rdy( std::move( f_rdy ) ), f_on_close( std::move( f_close ) ) {
}

UdpConnectionDTLS_WF::~UdpConnectionDTLS_WF() {
    if ( f_on_close ) f_on_close( this );
}

bool UdpConnectionDTLS_WF::wait_for_inp_at_beg() const {
    return bool( f_parse );
}

void UdpConnectionDTLS_WF::parse( char **data, size_t size ) {
    if ( f_parse ) f_parse( this, data, size );
}

void UdpConnectionDTLS_WF::on_rdy() {
    if ( f_on_rdy ) f_on_rdy( this );
}

} // namespace Evel
