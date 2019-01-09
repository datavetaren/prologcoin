#pragma once

#ifndef _pow_camera_hpp
#define _pow_camera_hpp

#include <vector>
#include <unordered_set>
#include <math.h>
#include "galaxy.hpp"
#include "star.hpp"

namespace prologcoin { namespace pow {

//
// It has a reference to the galaxy we're observing using
// our camera which has position and target vectors.
//

class camera_properties {
public:
    static const int32_t SCREEN_WIDTH = 1 << 12;
    static const int32_t SCREEN_HEIGHT = 1 << 12;
    static const int32_t SCREEN_DEPTH = 1 << 12;

    inline int32_t width() const {
	return SCREEN_WIDTH;
    }

    inline int32_t height() const {
	return SCREEN_HEIGHT;
    }

    inline int32_t depth() const {
	return SCREEN_DEPTH;
    }
};

template<size_t NumBits, typename T> class camera : public camera_properties {
public:
    static const size_t N = galaxy<NumBits,T>::num_buckets;

    inline camera(galaxy<NumBits, T> &galaxy, size_t id)
        : VOLUME(T(1)*32/T(N)/T(N)/T(N)), // Volume in standard units
	  galaxy_(galaxy), id_(id), origin_(0,0,0), target_(T(1)/10,0,0) {
    }

    inline size_t id() const {
	return id_;
    }

    inline const vec3<T> & get_origin() const {
	return origin_;
    }

    inline void set_target(T d, T th, T phi) {
	target_ = get_origin() + vec3<T>(vec3<T>::SPHERICAL(), d, th, phi);
    }

    inline void set_target(const vec3<T> &v) {
	target_ = get_origin() + v;
    }

    inline const vec3<T> & get_target() const {
	return target_;
    }


    // Set target direction of camera based on 'index' (we'll use a function
    // based on siphash). Then mask the resulting value and apply the fixed
    // value (sets of bits that should have a constant value.)

    inline void set_target(uint64_t nonce_offset, uint32_t nonce) {
	uint64_t out[3];
	uint64_t nonce_begin = nonce_offset + 3*nonce;
	uint64_t nonce_end = nonce_begin + 3;
	siphash(galaxy_.keys(), nonce_begin, nonce_end, out);
	set_target(vec3<T>(uint64_to_T<T>(out[0]), uint64_to_T<T>(out[1]), uint64_to_T<T>(out[2])));
    }

    // Contains some code duplication, but it's to make it easier for
    // verification without sacrificing some performance.
    inline bool project_star(size_t id, projected_star &star) const {
	static const T c_1(1);
	static const T c_1_2 = T(1)/2;
	static const T c_minus_1_2 = -(T(1)/2);

	auto st = galaxy_.get_star(id);
	auto sv = galaxy_.star_to_vector(st);
	auto svd = sv - get_origin(); // Star relative from origin
	auto sd = svd.length(); // Star distance from origin
	auto diff = get_target() - get_origin();
	auto dir = diff.norm();
	auto h = diff.length();
	T    l = sqrt(VOLUME * 3 / h);
	auto sdl_inv = h / (l * sd);  // Expected base pyramid size

	auto proxy_up = vec3<T>(0,0,1);
	auto dir_1 = dir.cross(proxy_up).norm();
	auto dir_2 = dir.cross(dir_1);

	auto proj_1 = svd.dot(dir_1) * sdl_inv;
	auto proj_2 = svd.dot(dir_2) * sdl_inv;
	auto proj_r = sd/h;
	
	// Is this star within the bounding box?
	// (Do not square to avoid overflows...)
	if (proj_1 >= c_minus_1_2 && proj_1 < c_1_2 &&
	    proj_2 >= c_minus_1_2 && proj_2 < c_1_2 &&
	    proj_r < c_1) {

	    // Normalize coordinates to 0..4095.
	    // Will be good enough to calculate visual observations
	    // and it'll be relatively easy to compute dx^2+dy^2
	    // without overflow.
	    uint32_t x32 = static_cast<uint32_t>((proj_1+c_1_2)*SCREEN_WIDTH);
	    uint32_t y32 = static_cast<uint32_t>((proj_2+c_1_2)*SCREEN_HEIGHT);
	    uint32_t r32 = static_cast<uint32_t>(proj_r*SCREEN_DEPTH);
	    star = projected_star(static_cast<uint32_t>(id), x32, y32, r32);
	    return true;
	} else {
	    return false;
	}
    }

    void take_picture(std::vector<projected_star> &stars) const;

private:
    const T VOLUME;

    galaxy<NumBits,T> &galaxy_;
    size_t id_;
    vec3<T> origin_;
    vec3<T> target_;
};

template<size_t NumBits, typename T> inline void camera<NumBits,T>::take_picture(std::vector<projected_star> &stars) const
{
    stars.clear();

    static const T c_1(1);

    auto origin = get_origin();
    auto target = get_target();
    auto diff = target - origin;
    auto dir = diff.norm();
    auto ds = galaxy_.step_vector_length();

    auto proxy_up = vec3<T>(0,0,1);
    auto dir_1 = dir.cross(proxy_up).norm();
    auto dir_2 = dir.cross(dir_1);
    auto NN = galaxy<NumBits,T>::num_buckets_bits;

    static const T c_1_2 = T(1)/2;
    static const T c_minus_1_2 = -(T(1)/2);
    size_t total_cnt = 0;

    // l: Base side of pyramid (our looking window is like a pyramid)
    // Volume of pyramid: V = l^2*h / 3 => l = sqrt(3*V/h)
    auto h = diff.length();
    T l = sqrt(VOLUME * 3 / h);

    //
    // Make a ray from the point of origin towards 'target' using
    // steps as specified by 'dir_step'. This is our ruler with markers
    // toward our destination.
    //
    //

    std::unordered_set<projected_star::id_type> star_ids;

    typedef std::tuple<uint64_t, uint64_t, uint64_t> key_t;

    struct key_hash {
	size_t operator()(const key_t& k) const {
	    return (std::get<0>(k) << 24) ^ (std::get<1>(k) << 16) ^ std::get<2>(k);
	}
    };


    std::unordered_set<key_t,key_hash> visited_;

    int num_cubes = 0;

    for (auto d = ds/2; d < h; d += ds/2) {

	auto p = origin + dir * d;
	auto dl = l * d / h; // Base pyramid size at this distance.

	// Render a grid in the plane of the direction of looking.
	for (auto y = -ds; y < dl+ds*2; y += ds/2) {
	    auto yd = dir_2*(y-dl/2);
	    for (auto x = -ds; x < dl+ds*2; x += ds/2) {
		auto xd = dir_1*(x-dl/2);
		auto pp = p + xd + yd;

		// (xp,yp,zp) is a point in this grid
		auto xp = pp.x();
		auto yp = pp.y();
		auto zp = pp.z();

		// Convert to uint64

		auto xp64 = T_to_uint64(xp);
		auto yp64 = T_to_uint64(yp);
		auto zp64 = T_to_uint64(zp);

		// Compute bucket 3D index

		auto xp64_i = xp64 >> (sizeof(uint64_t)*8-NN);
		auto yp64_i = yp64 >> (sizeof(uint64_t)*8-NN);
		auto zp64_i = zp64 >> (sizeof(uint64_t)*8-NN);

		auto key = std::make_tuple(xp64_i, yp64_i, zp64_i);
		if (visited_.find(key) != visited_.end()) {
		    continue;
		}

		visited_.insert(key);

		num_cubes++;

		// Iterate through all stars in this bucket (= octant)
		
		for (auto id : galaxy_.get_stars_ids(xp64_i, yp64_i, zp64_i) ) {
		    total_cnt++;
		    if (star_ids.find(id) != star_ids.end()) {
			continue;
		    }
		    star_ids.insert(id);
		    auto st = galaxy_.get_star(id);
		    auto sv = galaxy_.star_to_vector(st);
		    auto svd = sv - origin; // Star relative from origin
		    auto sd = svd.length(); // Star distance from origin
		    auto sdl_inv = h / (l * sd);  // Expected base pyramid size

		    auto proj_1 = svd.dot(dir_1) * sdl_inv;
		    auto proj_2 = svd.dot(dir_2) * sdl_inv;
		    auto proj_r = sd/h;

		    // Is this star within the bounding box?
		    // (Do not square to avoid overflows...)
		    if (proj_1 >= c_minus_1_2 && proj_1 < c_1_2 &&
			proj_2 >= c_minus_1_2 && proj_2 < c_1_2 &&
			proj_r < c_1) {

			// Normalize coordinates to 0..4095.
			// Will be good enough to calculate visual observations
			// and it'll be relatively easy to compute dx^2+dy^2
			// without overflow.
			uint32_t x32 = static_cast<uint32_t>((proj_1+c_1_2)*SCREEN_WIDTH);
			uint32_t y32 = static_cast<uint32_t>((proj_2+c_1_2)*SCREEN_HEIGHT);
			uint32_t r32 = static_cast<uint32_t>(proj_r*SCREEN_DEPTH);
			projected_star projected(st.id(), x32, y32, r32);

			stars.push_back(projected);
		    }
		}
	    }
	}
    }
    // std::cout << "Total count: " << total_cnt << std::endl;
    // std::cout << "Unique count: " << star_ids.size() << std::endl;
    // std::cout << "Projected: " << stars.size() << " " << target << std::endl;
    // std::cout << "Num cubes processed: " << num_cubes << std::endl;
}



}}

#endif
