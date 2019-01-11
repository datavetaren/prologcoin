#include <iostream>
#include "dipper_detector.hpp"
#include "isqrt.hpp"
#include "camera.hpp"
#include <fstream>
#include "pow_verifier.hpp"


#ifndef DIPPER_DONT_USE_NAMESPACE
namespace prologcoin { namespace pow {
#endif

dipper_detector::dipper_detector(std::vector<projected_star> &stars) : stars_(stars) {
}

//
// In Eucliedean geometry there's a finite number of relations among N
// points, which are invariant under translation/rotation. Specifically,
// the mututal distances between them is all what counts.
//

void dipper_detector::sort() {
    for (size_t gx = 0; gx < GRID_SIZE; gx++) {
	for (size_t gy = 0; gy < GRID_SIZE; gy++) {
	    sorted_[gx][gy].clear();
	}
    }
    
    for (auto &s : stars_) {
	size_t grid_x = (s.x() * GRID_SIZE) / camera_properties::SCREEN_WIDTH;
	size_t grid_y = (s.y() * GRID_SIZE) / camera_properties::SCREEN_HEIGHT;
	sorted_[grid_x][grid_y].push_back(s);
    }
}

bool dipper_detector::find_star(int32_t dx0, int32_t dy0, int32_t dr0, int32_t tol, int32_t tol_depth, projected_star &found_star)
{
    int32_t dx1 = (dx0*100 - tol) / 100;
    int32_t dx2 = (dx0*100 + tol) / 100;
    int32_t dy1 = (dy0*100 - tol) / 100;    
    int32_t dy2 = (dy0*100 + tol) / 100;

    int32_t r2 = tol*tol;
    // int32_t d2 = tol_depth*tol_depth;
    for (int32_t dx = dx1; dx < dx2; dx += GRID_SIZE) {
	for (int32_t dy = dy1; dy < dy2; dy += GRID_SIZE) {
	    if (dx >= 0 && dx < camera_properties::SCREEN_WIDTH &&
		dy >= 0 && dy < camera_properties::SCREEN_HEIGHT) {
		int32_t gx = to_grid_x(dx);
		int32_t gy = to_grid_y(dy);
		for (auto &s : sorted_[gx][gy]) {
		    int32_t diff_x = static_cast<int32_t>(s.x()) - dx0;
		    int32_t diff_y = static_cast<int32_t>(s.y()) - dy0;
		    // int32_t diff_r = static_cast<int32_t>(s.r()) - dr0;
		    if (100*(diff_x*diff_x + diff_y*diff_y) <= r2) {
			//if (diff_r*diff_r <= d2) {
		    	    found_star = s;
			    return true;
		        //}
		    }
		}
	    }
	}
    }
    return false;
}


bool dipper_detector::search_n(std::vector<std::vector<projected_star> > &found, size_t n)
{
    sort();

    //
    // Finding a star constellation is a non-trivial problem. There are
    // many combinations of stars that would fit the pattern if we allow
    // translational, scaling and rotational invariance.
    //

    // The location of the stars (as projected):
    //
    //                                  7
    //           2
    //                 3     4
    //   1                               6
    //                            5
    //
    // From this we can infer the default coordinate system, e.g. using
    // the stars (1)-(2). Then for every pair of stars we assume we're
    // seeing its (1)-(2) combination, for which we can compute the
    // relative orientation and scaling of its coordinate system. The
    // other stars can then be related using this coordinate system.
    //

    static const size_t N = 7;    
    static const int32_t dipper_dx[N] = { 0, 185, 325, 502, 588, 830, 805 };
    static const int32_t dipper_dy[N] = { 0, 116,  99,  93, -30,  63, 236 };
    static const int32_t dipper_dr[N] = { 9,   9,   9,  29,   9,  16,   9 };
	    
    const int32_t dx12 = 185;
    const int32_t dy12 = 116;
    const int32_t dl12 = isqrt(dx12*dx12+dy12*dy12);

    static size_t best = 0;

    bool success = false;
    size_t num_stars = stars_.size();

    std::vector<projected_star> found1;
	    
    for (size_t i = 0; i < num_stars && !success; i++) {
	int32_t xi = stars_[i].x(), yi = stars_[i].y();
	for (size_t j = 0; j < num_stars && !success; j++) {
	    if (i == j) continue;
	    int32_t xj = stars_[j].x(), yj = stars_[j].y();
	    int32_t dxij = xj-xi, dyij = yj-yi;
	    const int32_t dlij2 = dxij*dxij+dyij*dyij;
	    // If the distance between the stars is too small (5 units) then
	    // continue.
	    if (dlij2 < 5*5) {
		continue;
   	    }

	    const int32_t dlij = isqrt(dlij2);
	    
	    // Compute rotation/scaling matrix, that takes a vector from
	    // the dipper default coordinate system to the placement
	    // of the coordinate system in the sky.
	    int32_t R_00 = (dx12*dxij+dy12*dyij) / dl12;
	    int32_t R_01 = (dy12*dxij-dx12*dyij) / dl12;
	    int32_t R_10 = -R_01;
	    int32_t R_11 = R_00;

	    // Scale tolerance with scaling
	    int32_t tol = pow_verifier::TOLERANCE*dlij/dl12;

	    found1.clear();
	    found1.push_back(stars_[i]);
	    found1.push_back(stars_[j]);
	    for (size_t k = 2; k < N; k++) {
		// Let's take the dipper points (in the default coordinate
		// system) and translate them to the sky's position using
		// the two selected stars as reference.
		int32_t dx1k = dipper_dx[k];
		int32_t dy1k = dipper_dy[k];
		int32_t dx = (R_00*dx1k + R_01*dy1k) / dl12 + xi;
		int32_t dy = (R_10*dx1k + R_11*dy1k) / dl12 + yi;
		int32_t dr = dipper_dr[k]*dlij/dl12;

		projected_star found_star;
		// Is there a point in this location within the tolerance?
		if (find_star(dx, dy, dr, tol, tol*20, found_star)) {
		    found1.push_back(found_star);
		    if (k > best) {
			// std::cout << "best " << k << std::endl;
			best = k;
	    	    }
		    
		    if (k == N - 1) {
			// std::cout << "Found it!" << std::endl;
			success = true;
			/*
			for (auto fnd : found1) {
			    auto xs = fnd.x();
			    auto ys = fnd.y();
			    std::cout << "id=" << fnd.id() << " x=" << xs << " y=" << ys << std::endl;
			}
			*/
		    }
		} else {
		    // No point in looking further...
		    break;
                }
	    }
	    if (success) {
		found.push_back(found1);
		if (found.size() < n) {
		    success = false; // Keep looking for more
		}
	    }
	}
    }

    return success;
}

bool dipper_detector::search(std::vector<projected_star> &found)
{
    std::vector<std::vector<projected_star> > found_n;
    if (search_n(found_n, 1)) {
	found = found_n[0];
        return true;
    } else {
	return false;
    }
}

#ifndef DIPPER_DONT_USE_NAMESPACE	
}}
#endif

