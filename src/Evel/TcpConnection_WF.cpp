#include "TcpConnection_WF.h"

namespace Evel {

TcpConnection_WF::~TcpConnection_WF() {
    if ( f_close ) f_close( this );
}

void TcpConnection_WF::on_rdy() {
    if ( f_rdy ) f_rdy( this );
}

void TcpConnection_WF::parse( char **data, size_t size, size_t rese ) {
    if ( f_parse ) f_parse( this, data, size, rese );
}

} // namespace Evel
