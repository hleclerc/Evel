#pragma once

#include "Listener.h"
#include <functional>

namespace Evel {

/**
*/
class Listener_WF : public Listener {
public:
    using TF_rdy        = std::function<void( Listener_WF *c )>;
    using TF_connection = std::function<void( Listener_WF *c, int fd, const InetAddress &addr )>;
    using TF_close      = std::function<void( Listener_WF *c )>;

    Listener_WF( unsigned port, bool need_wait = true );
    ~Listener_WF();

    TF_rdy           f_rdy;
    TF_connection    f_connection;
    TF_close         f_close;

protected:
    virtual void     on_rdy    () override;
    virtual void     connection( int fd, const InetAddress &addr ) override;
};


} // namespace Evel
