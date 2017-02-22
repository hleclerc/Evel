#include "TlsConnection_WF.h"

namespace Evel {

TlsConnection_WF::~TlsConnection_WF() {
    if ( f_close ) f_close( this );
}

void TlsConnection_WF::on_rdy() {
    if ( f_rdy ) f_rdy( this );
}

void TlsConnection_WF::parse( char **data, size_t size, size_t rese ) {
    if ( f_parse ) f_parse( this, data, size, rese );
}

} // namespace Evel
