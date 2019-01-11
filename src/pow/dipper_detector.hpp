#pragma once
#ifndef _pow_dipper_detector_hpp
#define _pow_dipper_detector_hpp

#include <vector>
#include "star.hpp"
#include "camera.hpp"

#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

class dipper_detector {
public:
    dipper_detector(std::vector<projected_star> &stars);

    bool search(std::vector<projected_star> &found);
    bool search_n(std::vector<std::vector<projected_star> > &found, size_t n);
    bool find_star(int32_t dx0, int32_t dy0, int32_t dr0, int32_t tol, int32_t tol_depth, projected_star &found);

private:
    static const size_t GRID_SIZE = 32;

    inline static size_t to_grid_x(int32_t x) {
	return static_cast<size_t>(x * GRID_SIZE / camera_properties::SCREEN_WIDTH);
    }
    inline static size_t to_grid_y(int32_t y) {
	return static_cast<size_t>(y * GRID_SIZE / camera_properties::SCREEN_HEIGHT);
    }

    void sort();

    std::vector<projected_star> &stars_;
    std::vector<projected_star> sorted_[GRID_SIZE][GRID_SIZE];
};

#ifndef DIPPER_DONT_USE_NAMESPACE
}}
#endif

#endif


