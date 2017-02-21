#pragma once

namespace Evel {
class WaitOut;
class EvLoop;

/**
*/
class Event {
public:
    struct VtableOnly {};

    Event( int fd = -1 );                       ///< fd = file descriptor
    Event( VtableOnly );                        ///< a constructor that does not assign any attribute (else than the vtable). Allows to do a new( ptr ) T to change _only_ the vtable (underlying type).
    virtual ~Event();

    // public attributes
    int              fd;                        ///< file descriptor
    EvLoop          *ev_loop;                   ///< the event_loop where `this` is registered

    // public methods
    virtual void     close       ();            ///< delete this after all the outputs has been seen
    virtual void     del         ();            ///< say that we want to force deletion, even if some or all of the outputs has not been seen
    void             reg_work    ();            ///<


protected:
    friend class     WaitOut;
    friend class     EvLoop;

    virtual void     sys_error   ();            ///< I/O error
    virtual void     error       ();            ///< generic error
    virtual void     close_fd    ();            ///<

    virtual void     on_inp      ();            ///< called when input data. returns true if we want more input
    virtual void     on_out      ();            ///< called when ready for output. returns true if we want more output
    virtual void     on_rdy      ();            ///< called when connection/event is ready
    virtual void     on_rd_hup   ();            ///< gracefully ended connection (but we try to play with it again)
    virtual void     on_hup      ();            ///< not so gracefully ended connection
    virtual void     work        ();            ///< called when "work to do" (@see reg_work, ...)
    virtual void     _on_rdy     ();            ///<
    void             __on_rdy    ();            ///< called after installation of this in the event loop
    virtual bool     need_wait   () const;      ///< true if ev_loop has to wait for `this` (to return)
    virtual bool     may_have_out() const;      ///< may have output data (to know if a SIOCOUTQ test is useful)
    virtual bool     out_are_sent() const = 0;  ///<

    // attributes
    Event           *next_wait_out;             ///< used in CheckSent (protected by EvLoop::mutex)
    Event           *next_work;                 ///< work to do
    Event           *next_del;                  ///<
    bool             del_on_install;            ///<
    bool             work_on_install;           ///<
    bool             want_close_fd;             ///<
    bool             has_error;                 ///<
};

} // namespace Evel
