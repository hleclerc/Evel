#pragma once

#include "EvLoop.h"

namespace Evel {

/// called after event loop checks
class PostFunc {
public:
    PostFunc() : ev_loop( 0 ), prev_post_func( 0 ) {}
    virtual ~PostFunc() {}

    virtual bool exec() = 0; ///< return true is something done need to loop over the other postfuncs

    EvLoop   *ev_loop;
    PostFunc *prev_post_func; ///< to allow chained lists of post funcs
};

} // namespace Evel
