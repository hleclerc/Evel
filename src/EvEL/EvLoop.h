#pragma once

#include "CiList.h"
#include "Event.h"
#include <atomic>

namespace Evel {

class WaitOut;
class Event;

/**
*/
class EvLoop {
public:
    EvLoop();
    ~EvLoop();

    int              run             ();                                       ///< Infinite loop, until this->stop() is called. return err code < 0 if pb. return ret_val (the value that is sent by this->stop()) if ok. wait_out_flushed is useful before exiting.
    bool             stop            ( int ret_val = 0 );                      ///< Procedure to be called to gracefully exit from run(). May be called e.g. during a callback. Return false for convenience

    EvLoop          &operator<<      ( Event *ev_obj );                        ///< add a event object that has to be watched. ev_obj may be deleted inside the loop if it says that everything is done with it. thread safe.
    EvLoop          &operator>>      ( Event *ev_obj );                        ///< suppression of an event object NOT thread safe

    void             add_work        ( Event *ev_obj );                        ///<

    virtual void     log             ( const char *msg, const char *cmp = 0 ); ///< method that may be surdefined for logging
    virtual void     err             ( const char *msg, const char *cmp = 0 ); ///< method that may be surdefined for error logging
    
protected:
    struct NO { Event *&operator()( Event *item ) const { return item->next_wait_out; } };
    struct NW { Event *&operator()( Event *item ) const { return item->next_work;     } };
    struct ND { Event *&operator()( Event *item ) const { return item->next_del;      } };
    friend class WaitOut;
    friend class Event;


    void             check_wait_out  ();                                       ///< check if there is an active wait_out_timer

    WaitOut         *wait_out_timer;                                           ///< it we are waiting for data to be (really) sent before closing the concerned sockets/files/...
    std::mutex       wait_out_mutex;                                           ///<
    CiList<Event,NO> wait_out_list;                                            ///< event with fd that still have stuff to flush (before closing the fd)
    CiList<Event,NW> work_list;                                                ///< event that have work to do
    CiList<Event,ND> del_list;                                                 ///< event that have have to be deleted
    std::atomic<int> nb_waiting_events;                                        ///<
    int              event_fd;                                                 ///< for epoll
    int              ret;                                                      ///< value run() will have to return
    bool             cnt;                                                      ///< continue ?
};

} // namespace Evel
