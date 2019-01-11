#pragma once

#ifndef _pow_galaxy_hpp
#define _pow_galaxy_hpp

#include "siphash.hpp"
#include "buckets.hpp"
#include "vec3.hpp"
#include "star.hpp"
#include "conv.hpp"
#include "checked_cast.hpp"

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

//
// galaxy. It's not really a "real" galaxy in the sense that the distribution
// of stars is random. A more accurate name would be a star cluster and even
// that is wrong as the cluster is not spherically distributed.
//
// Nevertheless, the galaxy as represented in normalized space from
// -0.5..+0.5 for (x,y,z), has S number of stars. The space is divided
// into 2^N buckets (or sub cubes) where a certain number of stars reside.
// This makes it simpler to prune the stars of interest given the position
// of the camera and the direction we're looking. The vision field can be
// represented as a pyramid with it's tip at the origin of the camera.
// By keeping the volume constant we see the same number of stars no matter
// what the "zoom" factor is. Thus, the pyramid gets steeper as the target
// point moves further away from the camera origin.
//

template<size_t NumBits, typename T> class galaxy {
public:
    static const size_t num_buckets_bits = NumBits;
    static const size_t num_buckets = 1 << num_buckets_bits;

    inline galaxy( const siphash_keys &keys )
	: keys_(keys) { }

    inline const siphash_keys & keys() const {
	return keys_;
    }

    inline star get_star(uint32_t id) const {
	uint64_t out[3];
	siphash(keys_, checked_cast<uint64_t>(3*id),
		       checked_cast<uint64_t>(3*id+3), out);
	return star(id, out[0], out[1], out[2]);
    }

    struct range_stars;
    friend struct range_stars;

    struct range_stars {
	inline range_stars(const galaxy &g, size_t x, size_t y, size_t z)
	    : g_(g), x_(x), y_(y), z_(z) { }
	inline bucket_iterator<uint32_t> begin() const
	    { return g_.stars_.get(x_,y_,z_).begin(); }
	inline bucket_iterator<uint32_t> end() const
	    { return g_.stars_.get(x_,y_,z_).end(); }

	const galaxy &g_;
	size_t x_, y_, z_;
    };

    inline range_stars get_stars_ids(size_t x, size_t y, size_t z) {
	return range_stars(*this, x, y, z);
    }

    inline vec3<T> star_to_vector(const star &s) const {
	return vec3<T>( uint64_to_T<T>(s.x()),
			uint64_to_T<T>(s.y()),
			uint64_to_T<T>(s.z()) );
    }

    inline void clear() {
	stars_.clear();
    }

    void init(size_t num_stars = 1 << (3*NumBits+3));
    void check();
    void memory() const;
    void status() const;

    inline T step_vector_length() const {
	return T(1) / num_buckets;
    }

private:
    static const size_t N = num_buckets;
    static const size_t NN = num_buckets_bits;

    class star_id {
    public:
	inline star_id(const star &s) : s_(s) { }

	inline uint32_t operator () () const { 
	    return s_.id();
	}
    private:
        const star &s_;
    };

    class star_x {
    public:
	inline star_x(const star &s) : s_(s) { }

	inline size_t operator () () const { 
	    return s_.x() >> (sizeof(uint64_t)*8-NN);
	}
    private:
        const star &s_;
    };

    class star_y {
    public:
	inline star_y(const star &s) : s_(s) { }

	inline size_t operator () () const { 
	    return s_.y() >> (sizeof(uint64_t)*8-NN);
	}
    private:
        const star &s_;
    };

    class star_z {
    public:
	inline star_z(const star &s) : s_(s) { }

	inline size_t operator () () const { 
	    return s_.z() >> (sizeof(uint64_t)*8-NN);
	}
    private:
        const star &s_;
    };

    const siphash_keys &keys_;
    size_t num_stars_;
    buckets<N, star_x, buckets<N, star_y, buckets<N, star_z, bucket<32, star_id, star> > > > stars_;
};

//
// 1 pc (parsec) = 3.26 light years
// 30000 pc = size of milky way
// Number of stars in milky way: 300e+9 (300 billion)
// Let's assume 30000 pc in dx, dy, dz (which is not correct as most stars
// are in a planar orbit)
// then density of stars : 300e+9 / 30000^3 pc^3 

template<size_t NumBits, typename T> void galaxy<NumBits,T>::init(size_t num_stars) 
{
    const size_t N = 32;
    uint64_t chunk[3*N];

    num_stars_ = num_stars;

    // std::cout << "galaxy<" << NumBits << ">::init(): num_stars=" << num_stars << std::endl;

    for (size_t i = 0; i < num_stars; i += N) {
	// if (i % (N*100000) == 0) {
	//    std::cout << "   currently=" << i << " stars...\r";
	// }
	size_t nn = std::min(N, num_stars-i);
	size_t i_end = i + nn;
	siphash(keys_, checked_cast<uint64_t>(3*i), checked_cast<uint64_t>(3*i_end), chunk);
	for (size_t j = 0; j < nn; j++) {
	    star s(i+j, chunk[j*3], chunk[j*3+1], chunk[j*3+2]);
	    // No luck in cheating...
	    // if (s.x() & ((uint64_t)(1) << 63)) {
	        stars_.put(s);
	    // }
	}
    }
    // std::cout << std::endl;
}

template<size_t NumBits,typename T> void galaxy<NumBits,T>::status() const {
    size_t cnt = 0;
    size_t max_bucket_size = 0;
    for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	    for (size_t k = 0; k < N; k++) {
		const auto &bucket = stars_.get(i,j,k);
		if (bucket.size() > max_bucket_size) {
		    max_bucket_size = bucket.size();
		}
		auto it_end = bucket.end();
		for (auto it = bucket.begin(); it != it_end; ++it) {
		    uint32_t id = *it;
		    auto s = get_star(id);
		    size_t x_bucket = checked_cast<size_t>(s.x() >> (64-NN));
	 	    size_t y_bucket = checked_cast<size_t>(s.y() >> (64-NN));
		    size_t z_bucket = checked_cast<size_t>(s.z() >> (64-NN));
		    assert(i == x_bucket);
		    assert(j == y_bucket);
		    assert(k == z_bucket);
		    cnt++;
		}
	    }
	}
    }
    size_t num_buckets = 0;
    size_t buckets_size_sum = 0;
    for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	    for (size_t k = 0; k < N; k++, num_buckets++) {
		const auto &bucket = stars_.get(i,j,k);
		buckets_size_sum += bucket.size();
	    }
	}
    }
    double avg_bucket_size = static_cast<double>(buckets_size_sum) / num_buckets;

    // std::cout << "Max bucket size: " << max_bucket_size << std::endl;
    // std::cout << "Avg bucket size: " << avg_bucket_size << std::endl;
    // std::cout << "Num stars: " << cnt << std::endl;
    // memory();
    (void)avg_bucket_size;
    (void)num_buckets;
    (void)buckets_size_sum;
    // assert(cnt == num_stars_);
}

template<size_t NumBits, typename T> void galaxy<NumBits,T>::memory() const {
    size_t n = stars_.memory();
    size_t mb = n / 1000000;
    std::cout << "Galaxy memory: " << mb << " MB" << std::endl;
}

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif
