#ifndef LIBNBT_CONSTANTS_HPP
#define LIBNBT_CONSTANTS_HPP

#include "../../nbtasm/nbtasm.hpp"

static const int MAX_SHORT_DISTANCE = 5;
static const int MAX_LONG_DISTANCE = 15;
static const int MAX_FAR_DISTANCE = 30;

namespace detail {

static std::array<Vec3, 18> enumerate_near_distances(){
	std::array<Vec3, 18> ret;
	for(int z = -1, k = 0; z <= 1; ++z){
		for(int y = -1; y <= 1; ++y){
			for(int x = -1; x <= 1; ++x){
				if(z != 0 && y != 0 && x != 0){ continue; }
				if(z == 0 && y == 0 && x == 0){ continue; }
				ret[k++] = Vec3(x, y, z);
			}
		}
	}
	return ret;
}

static std::array<Vec3, 30> enumerate_short_distances(){
	std::array<Vec3, 30> ret;
	int k = 0;
	for(int x = -MAX_SHORT_DISTANCE; x <= MAX_SHORT_DISTANCE; ++x){
		if(x != 0){ ret[k++] = Vec3(x, 0, 0); }
	}
	for(int y = -MAX_SHORT_DISTANCE; y <= MAX_SHORT_DISTANCE; ++y){
		if(y != 0){ ret[k++] = Vec3(0, y, 0); }
	}
	for(int z = -MAX_SHORT_DISTANCE; z <= MAX_SHORT_DISTANCE; ++z){
		if(z != 0){ ret[k++] = Vec3(0, 0, z); }
	}
	return ret;
}

static std::array<Vec3, 90> enumerate_long_distances(){
	std::array<Vec3, 90> ret;
	int k = 0;
	for(int x = -MAX_LONG_DISTANCE; x <= MAX_LONG_DISTANCE; ++x){
		if(x != 0){ ret[k++] = Vec3(x, 0, 0); }
	}
	for(int y = -MAX_LONG_DISTANCE; y <= MAX_LONG_DISTANCE; ++y){
		if(y != 0){ ret[k++] = Vec3(0, y, 0); }
	}
	for(int z = -MAX_LONG_DISTANCE; z <= MAX_LONG_DISTANCE; ++z){
		if(z != 0){ ret[k++] = Vec3(0, 0, z); }
	}
	return ret;
}

}

static const auto NEAR_DISTANCE_TABLE = detail::enumerate_near_distances();
static const auto SHORT_DISTANCE_TABLE = detail::enumerate_short_distances();
static const auto LONG_DISTANCE_TABLE = detail::enumerate_long_distances();

#endif
