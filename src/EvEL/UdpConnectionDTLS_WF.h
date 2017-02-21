#pragma once

#include "UdpConnectionDTLS.h"
#include <functional>

namespace Evel {

/**
*/
class UdpConnectionDTLS_WF : public UdpConnectionDTLS {
public:
    using TF_parse    = std::function<void( UdpConnectionDTLS_WF *conn, char **data, size_t size )>;
    using TF_on_rdy   = std::function<void( UdpConnectionDTLS_WF *conn )>;
    using TF_on_close = std::function<void( UdpConnectionDTLS_WF *conn )>;

    UdpConnectionDTLS_WF( SSL_CTX *ctx, bool server, UdpConnectionFactory *ucf, const InetAddress &addr, TF_parse &&f_parse = {}, TF_on_rdy &&f_on_rdy = {}, TF_on_close &&f_on_close = {} );
    ~UdpConnectionDTLS_WF();

    TF_parse              f_parse;
    TF_on_rdy             f_on_rdy;
    TF_on_close           f_on_close;
    double                v_timeout_read;

protected:
    virtual bool          wait_for_inp_at_beg() const; ///< true if completed at least after one input
    virtual void          parse              ( char **data, size_t size );
    virtual void          on_rdy             (); ///< called when the connection is up and running
    virtual double        timeout_read       () const; ///< nb seconds to wait before deleting this if no answer
};

} // namespace Evel
