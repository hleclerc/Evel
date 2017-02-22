#pragma once

namespace Evel {

template<class T>
struct RcPtr {
    RcPtr() : data( 0 ) {}
    RcPtr( T *obj ) : data( obj ) {}
    RcPtr( RcPtr &&obj ) : data( obj.data ) { obj.data = 0; }
    RcPtr( const RcPtr &obj ) : data( obj.data ) { if ( data ) ++data->cpt_use; }

    template<class U>
    RcPtr( const RcPtr<U> &obj ) : data( obj.data ) { if ( data ) ++data->cpt_use; }

    template<class U>
    RcPtr( RcPtr<U> &&obj ) : data( obj.data ) { obj.data = 0; }

    ~RcPtr() {
        if ( data ) {
            if ( data->cpt_use )
                --data->cpt_use;
            else
                delete data;
        }
    }

    RcPtr &operator=( T *obj ) {
        if ( data ) {
            if ( data->cpt_use )
                --data->cpt_use;
            else
                delete data;
        }
        data = obj;
        return *this;
    }

    template<class U>
    RcPtr &operator=( U *obj ) {
        if ( data ) {
            if ( data->cpt_use )
                --data->cpt_use;
            else
                delete data;
        }
        data = obj;
        return *this;
    }

    RcPtr &operator=( const RcPtr &obj ) {
        if ( obj.data )
            ++obj.data->cpt_use;
        if ( data ) {
            if ( data->cpt_use )
                --data->cpt_use;
            else
                delete data;
        }
        data = obj.data;
        return *this;
    }

    template<class U>
    RcPtr &operator=( const RcPtr<U> &obj ) {
        if ( obj.data )
            ++obj.data->cpt_use;
        if ( data ) {
            if ( data->cpt_use )
                --data->cpt_use;
            else
                delete data;
        }
        data = obj.data;
        return *this;
    }

    RcPtr &operator=( RcPtr &&obj ) {
        data = obj.data;
        obj.data = 0;
        return *this;
    }

    template<class U>
    RcPtr &operator=( RcPtr<U> &&obj ) {
        data = obj.data;
        obj.data = 0;
        return *this;
    }


    operator bool() const { return data; }

    void clear() { if ( data ) { delete data; data = nullptr; } }

    bool operator==( const T           *p ) const { return data == p;      }
    bool operator==( const RcPtr<T>    &p ) const { return data == p.data; }
    // bool operator==( const ConstPtr<T> &p ) const { return data == p.data; }

    bool operator!=( const T           *p ) const { return data != p;      }
    bool operator!=( const RcPtr<T>    &p ) const { return data != p.data; }
    // bool operator!=( const ConstPtr<T> &p ) const { return data != p.data; }

    bool operator< ( const T           *p ) const { return data <  p;      }
    bool operator< ( const RcPtr<T>    &p ) const { return data <  p.data; }
    // bool operator< ( const ConstPtr<T> &p ) const { return data <  p.data; }

    bool operator<=( const T           *p ) const { return data <= p;      }
    bool operator<=( const RcPtr<T>    &p ) const { return data <= p.data; }
    // bool operator<=( const ConstPtr<T> &p ) const { return data <= p.data; }

    bool operator> ( const T           *p ) const { return data >  p;      }
    bool operator> ( const RcPtr<T>    &p ) const { return data >  p.data; }
    // bool operator> ( const ConstPtr<T> &p ) const { return data >  p.data; }

    bool operator>=( const T           *p ) const { return data >= p;      }
    bool operator>=( const RcPtr<T>    &p ) const { return data >= p.data; }
    // bool operator>=( const ConstPtr<T> &p ) const { return data >= p.data; }

    const T *ptr() const { return data; }
    T *ptr() { return data; }

    const T *operator->() const { return data; }
    T *operator->() { return data; }

    const T &operator*() const { return *data; }
    T &operator*() { return *data; }

    template<class Os>
    void write_to_stream( Os &os ) const {
        if ( data )
            os << *data;
        else
            os << "NULL";
    }

    T *data;
};

template<class T>
inline const T *inc_ref( const T *p ) {
    ++p->cpt_use;
    return p;
}

template<class T>
inline T *inc_ref( T *p ) {
    ++p->cpt_use;
    return p;
}

template<class T>
inline void dec_ref( const T *ptr ) {
    if ( --ptr->cpt_use < 0 )
        delete ptr;
}

} // namespace Evel
