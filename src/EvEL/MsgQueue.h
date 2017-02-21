#pragma once

#include "TypeConfig.h"
#include "Print.h"

namespace Evel {

/**
*/
class MsgQueue {
public:
    /// a single message
    struct Msg {
        static Msg *New            ( unsigned size );
        void        write_to_stream( std::ostream &os ) const;

        Msg        *next;
        unsigned    size;      ///< real size of data[]
        PI8         data[ 4 ]; ///<
    };
    enum {
        header_size = sizeof( Msg ) - 4
    };

    MsgQueue();
    ~MsgQueue();

    void write_to_stream( std::ostream &os ) const;
    void operator<<     ( Msg *msg );
    bool empty          () const { return not first; }

    Msg *first;
    Msg *last;
};

} // namespace Evel
