#include "UdpConnectionFactory.h"
#include "UdpConnection.h"
#include "EvLoop.h"
#include <string.h>

namespace Evel {

UdpConnection::UdpConnection( UdpConnectionFactory *ucf, const InetAddress &addr ) : ucf( ucf ), addr( addr ) {
    next_del  = 0;
    has_error = false;
}

UdpConnection::~UdpConnection() {
}

void UdpConnection::close() {
    del();
}

void UdpConnection::del() {
    ucf->to_del.push_back( this );
    if ( ucf->ev_loop )
        ucf->ev_loop->add_work( ucf );
    else
        ucf->work_on_install = true;
}

void UdpConnection::send( const char *data ) {
    send( data, strlen( data ) );
}

void UdpConnection::send( const char *data, size_t size ) {
    send( &data, size, false );
}

void UdpConnection::send( const char **data, size_t size, bool allow_transfer_ownership ) {
    send_raw( data, size, allow_transfer_ownership );
}

void UdpConnection::_parse( char **data, size_t size ) {
    parse( data, size );
}

void UdpConnection::send_raw( const char **data, size_t size, bool allow_transfer_ownership ) {
    ucf->send_raw( addr, data, size, allow_transfer_ownership );
}

bool UdpConnection::send_avail_at_beg() const {
    return true;
}

void UdpConnection::sys_error( const char *ctx ) {
    perror( ctx ? ctx : "sys_error" );
    del();
}

void *UdpConnection::msg_alloc( size_t size ) {
    return malloc( size );
}

void UdpConnection::msg_free( const void *ptr ) {
    free( (void *)ptr );
}


} // namespace Evel
