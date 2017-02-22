#pragma once

#include "ExpIndexedList.h"
#include "CiList.h"
#include "Event.h"
#include <atomic>

namespace Evel {

class TimeoutsTimer;
class WaitOut;
class Event;

/**
*/
class EvLoop {
public:
    EvLoop();
    ~EvLoop();

    int              run                  ();                                                      ///< Infinite loop, until this->stop() is called. return err code < 0 if pb. return ret_val (the value that is sent by this->stop()) if ok. wait_out_flushed is useful before exiting.
    bool             stop                 ( int ret_val = 0 );                                     ///< Procedure to be called to gracefully exit from run(). May be called e.g. during a callback. Return false for convenience

    EvLoop          &add_event_obj        ( Event *ev_obj );                                       ///< add a event object that has to be watched. ev_obj may be deleted inside the loop if it says that everything is done with it. thread safe.
    EvLoop          &rem_event_obj        ( Event *ev_obj );                                       ///< suppression of an event object NOT thread safe

    EvLoop          &operator<<           ( Event *ev_obj );                                       ///< synonym for add_event_obj()
    EvLoop          &operator>>           ( Event *ev_obj );                                       ///< synonym for rem_event_obj()

    void             add_work             ( Event *ev_obj );                                       ///< ev_obj->work will be called in the "work" part of the event loop

    void             add_timeout          ( TimeoutEvent *ev_obj, double delay );                  ///< (if not suppressed from the list) ev_obj->on_timeout() will be called when delay expires
    void             rem_timeout          ( TimeoutEvent *ev_obj );

    virtual void     log                  ( const char *msg, const char *cmp = 0 );                ///< method that may be surdefined for logging
    virtual void     err                  ( const char *msg, const char *cmp = 0 );                ///< method that may be surdefined for error logging

    void             log                  ( const std::string &msg, const std::string &cmp = {} ); ///< method that may be surdefined for logging
    void             err                  ( const std::string &msg, const std::string &cmp = {} ); ///< method that may be surdefined for error logging

    bool             running              () const;

    void             inc_nb_waiting_events() { ++nb_waiting_events; }
    void             dec_nb_waiting_events() { --nb_waiting_events; }

protected:
    struct NO { Event *&operator()( Event *item ) const { return item->next_wait_out; } };
    struct NW { Event *&operator()( Event *item ) const { return item->next_work;     } };
    struct ND { Event *&operator()( Event *item ) const { return item->next_del;      } };
    using TimeoutList = ExpIndexedList<TimeoutEvent,TimeoutEvent::GetTimeoutData,64,3>;       ///< shifted every 0.125s => ~9h of indexed events
    friend class TimeoutsTimer;
    friend class WaitOut;
    friend class Event;


    void             check_wait_out  ();                                                      ///< check if there is an active wait_out_timer

    std::mutex       wait_out_mutex;                                                          ///<
    WaitOut         *wait_out_timer;                                                          ///< it we are waiting for data to be (really) sent before closing the concerned sockets/files/...
    TimeoutsTimer   *timeouts_timer;                                                          ///< it we are waiting for data to be (really) sent before closing the concerned sockets/files/...
    TimeoutList      timeout_list;
    CiList<Event,NO> wait_out_list;                                                           ///< event with fd that still have stuff to flush (before closing the fd)
    CiList<Event,NW> work_list;                                                               ///< event that have work to do
    CiList<Event,ND> del_list;                                                                ///< event that have have to be deleted
public:
    std::atomic<int> nb_waiting_events;                                                       ///<
    int              event_fd;                                                                ///< for epoll
    int              ret;                                                                     ///< value run() will have to return
    bool             cnt;                                                                     ///< continue ?
};

} // namespace Evel
