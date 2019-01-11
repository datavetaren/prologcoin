#pragma once

#ifndef _pow_buckets_hpp
#define _pow_buckets_hpp

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

template<size_t N, typename P, typename T> class buckets {
public:
    typedef typename T::value_type value_type;
    typedef typename T::bucket_type bucket_type;
    using V = value_type;

    inline buckets() { buckets_ = new T[N]; }

    inline void clear() {
	for (size_t i = 0; i < N; i++) {
	    buckets_[i].clear();
	}
    }

    inline void put(const V &v) {
	buckets_[P(v)()].put(v);
    }

    inline size_t memory() const {
	size_t n =  N * sizeof(T);
	for (size_t i = 0; i < N; i++) {
	    n += buckets_[i].memory();
	}
	return n;
    }

    template<typename ... Ts> inline bucket_type get(size_t arg, Ts... args) const {
	return buckets_[arg].get(args...);
    }
private:
    T *buckets_;
};

template<typename V> class bucket_iterator : public std::iterator<std::forward_iterator_tag, V, V, const V *, const V &> {
public:
    bucket_iterator(V *v) : v_(v) { }

    inline bucket_iterator & operator ++ () {
	v_++;
	return *this;
    }

    inline const V & operator * () const {
	return *v_;
    }

    inline bool operator == (const bucket_iterator<V> &other) const {
	return v_ == other.v_;
    }

    inline bool operator != (const bucket_iterator<V> &other) const {
	return v_ != other.v_;
    }
private:
    V *v_;
};

template<size_t N, typename P, typename T> class bucket {
public:
    typedef T value_type;
    typedef typename T::index_type index_type;
    typedef bucket<N,P,T>  bucket_type;

    inline bucket() : bucket_(new index_type[N]), sz_(0) { }

    inline void clear() {
	sz_ = 0;
    }

    inline void put(const T &v) {
	assert(sz_ < N);
	bucket_[sz_++] = P(v)();
    }

    inline const bucket<N,P,T> & get() const {
	return *this;
    }

    inline size_t size() const {
	return sz_;
    }

    inline size_t memory() const {
	return sizeof(index_type)*N;
    }

    inline bucket_iterator<index_type> begin() const { return bucket_iterator<index_type>(bucket_); }
    inline bucket_iterator<index_type> end() const { return bucket_iterator<index_type>(bucket_+sz_);}

private:
    index_type *bucket_;
    size_t sz_;
};

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
