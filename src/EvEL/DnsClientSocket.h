#pragma once

#include "UdpSocket.h"

namespace Evel {

class Dns;

/**
*/
class DnsClientSocket : public UdpSocket {
public:
    DnsClientSocket( Dns *dns );

protected:
    virtual void parse( const InetAddress &src, char **data, unsigned size ) override;

    Dns *dns;
};


} // namespace Evel
