#include "UdpSocket_WF.h"

namespace Evel {

UdpSocket_WF::~UdpSocket_WF() {
    if ( f_close ) f_close( this );
}

void UdpSocket_WF::on_rdy() {
    if ( f_rdy ) f_rdy( this );
}

void UdpSocket_WF::parse( const InetAddress &src, char **data, unsigned size ) {
    if ( f_parse ) f_parse( this, src, data, size );
}

void *UdpSocket_WF::allocate( size_t inp_buf_size ) {
    return f_allocate ? f_allocate( this, inp_buf_size ) : UdpSocket::allocate( inp_buf_size );
}

void UdpSocket_WF::on_bind_error( const char *msg ) {
    return f_on_bind_error ? f_on_bind_error( this, msg ) : UdpSocket::on_bind_error( msg );
}

} // namespace Evel
