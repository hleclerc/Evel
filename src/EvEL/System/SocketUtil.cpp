#include "SocketUtil.h"
#include <fcntl.h>
#include <errno.h>

namespace Evel {

int set_non_block( int fd ) {
    // get flags
    int r;
    do
      r = fcntl( fd, F_GETFL );
    while ( r == -1 && errno == EINTR );
    if ( r == -1 )
      return -errno;

    // already non blocking ?
    if ( r & O_NONBLOCK )
      return 0;

    // update flags
    int flags = r | O_NONBLOCK;
    do
      r = fcntl( fd, F_SETFL, flags );
    while ( r == -1 && errno == EINTR );

    return r ? -errno : 0;
}

} // namespace Evel

