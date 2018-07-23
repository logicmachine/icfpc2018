#ifndef LIBNBT_NO_CONFLICT_HPP
#define LIBNBT_NO_CONFLICT_HPP

#include "../../nbtasm/nbtasm.hpp"
#include "constants.hpp"

inline bool dispatch_move_no_conflict(State& s, int index, const Vec3& target){
	const auto cur = s.bots(index).pos();
	if(cur == target){ return true; }
	const auto d = target - cur;
	if(d.x < -MAX_SHORT_DISTANCE){ s.bots(index).smove(Vec3(std::max(d.x, -MAX_LONG_DISTANCE), 0, 0)); return false; }
	if(d.x >  MAX_SHORT_DISTANCE){ s.bots(index).smove(Vec3(std::min(d.x,  MAX_LONG_DISTANCE), 0, 0)); return false; }
	if(d.y < -MAX_SHORT_DISTANCE){ s.bots(index).smove(Vec3(0, std::max(d.y, -MAX_LONG_DISTANCE), 0)); return false; }
	if(d.y >  MAX_SHORT_DISTANCE){ s.bots(index).smove(Vec3(0, std::min(d.y,  MAX_LONG_DISTANCE), 0)); return false; }
	if(d.z < -MAX_SHORT_DISTANCE){ s.bots(index).smove(Vec3(0, 0, std::max(d.z, -MAX_LONG_DISTANCE))); return false; }
	if(d.z >  MAX_SHORT_DISTANCE){ s.bots(index).smove(Vec3(0, 0, std::min(d.z,  MAX_LONG_DISTANCE))); return false; }
	if(d.y == 0 && d.z == 0){ s.bots(index).smove(d); return false; }
	if(d.x == 0 && d.z == 0){ s.bots(index).smove(d); return false; }
	if(d.x == 0 && d.y == 0){ s.bots(index).smove(d); return false; }
	if(d.x != 0 && d.y != 0){ s.bots(index).lmove(Vec3(d.x, 0, 0), Vec3(0, d.y, 0)); return false; }
	if(d.x != 0 && d.z != 0){ s.bots(index).lmove(Vec3(d.x, 0, 0), Vec3(0, 0, d.z)); return false; }
	if(d.y != 0 && d.z != 0){ s.bots(index).lmove(Vec3(0, d.y, 0), Vec3(0, 0, d.z)); return false; }
	return false;
}

#endif
