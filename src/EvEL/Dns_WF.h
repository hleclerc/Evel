#pragma once

#include <functional>
#include "Dns.h"

namespace Evel {

/**
*/
class Dns_WF : public Dns {
public:
    using TF_Dns = std::function<void( Dns_WF *, int sig )>;

    Dns_WF( const int *sigs, TF_dns &&f = {} );

    TF_Dns f_dns;

protected:
    virtual void Dns( int s );
};

} // namespace Evel
