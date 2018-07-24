#ifndef LIBNBT_COLLECT_NANOBOTS_HPP
#define LIBNBT_COLLECT_NANOBOTS_HPP

#include "../../nbtasm/nbtasm.hpp"
#include "interference_checker.hpp"

void collect_nanobots_x(State& s){
	while(true){
		bool done_flag = true;
		for(int i = 0; i < s.num_bots(); ++i){
			const auto p = s.bots(i).pos();
			if(p.x > 0){ done_flag = false; }
		}
		if(done_flag){ break; }
		InterferenceChecker initial_ic(s);
		InterferenceChecker move_ic(s);
		for(int i = 0; i < s.num_bots(); ++i){
			const auto p = s.bots(i).pos();
			if(p.x == 0 && !initial_ic.test(Vec3(1, p.y, p.z))){
				s.bots(i).fusion_p(Vec3( 1, 0, 0));
			}else if(p.x == 1 && !initial_ic.test(Vec3(0, p.y, p.z))){
				s.bots(i).fusion_s(Vec3(-1, 0, 0));
			}else{
				int target = p.x;
				while(target > 0 && move_ic.set(Vec3(target - 1, p.y, p.z))){ --target; }
				s.bots(i).smove(Vec3(std::max(target - p.x, -MAX_LONG_DISTANCE), 0, 0));
			}
		}
		s.commit();
	}
}

void collect_nanobots_y(State& s){
	while(true){
		bool done_flag = true;
		for(int i = 0; i < s.num_bots(); ++i){
			const auto p = s.bots(i).pos();
			if(p.y > 0){ done_flag = false; }
		}
		if(done_flag){ break; }
		InterferenceChecker initial_ic(s);
		InterferenceChecker move_ic(s);
		for(int i = 0; i < s.num_bots(); ++i){
			const auto p = s.bots(i).pos();
			if(p.y == 0 && !initial_ic.test(Vec3(p.x, 1, p.z))){
				s.bots(i).fusion_p(Vec3(0,  1, 0));
			}else if(p.y == 1 && !initial_ic.test(Vec3(p.x, 0, p.z))){
				s.bots(i).fusion_s(Vec3(0, -1, 0));
			}else{
				int target = p.y;
				while(target > 0 && move_ic.set(Vec3(p.x, target - 1, p.z))){ --target; }
				s.bots(i).smove(Vec3(0, std::max(target - p.y, -MAX_LONG_DISTANCE), 0));
			}
		}
		s.commit();
	}
}

void collect_nanobots_y_inv(State& s){
	const int n = s.matrix().size();
	while(true){
		bool done_flag = true;
		for(int i = 0; i < s.num_bots(); ++i){
			const auto p = s.bots(i).pos();
			if(p.y < n - 1){ done_flag = false; }
		}
		if(done_flag){ break; }
		InterferenceChecker initial_ic(s);
		InterferenceChecker move_ic(s);
		for(int i = 0; i < s.num_bots(); ++i){
			const auto p = s.bots(i).pos();
			if(p.y == 0 && !initial_ic.test(Vec3(p.x, n - 2, p.z))){
				s.bots(i).fusion_p(Vec3(0, -1, 0));
			}else if(p.y == 1 && !initial_ic.test(Vec3(p.x, n - 1, p.z))){
				s.bots(i).fusion_s(Vec3(0,  1, 0));
			}else{
				int target = p.y;
				while(target < n - 1 && move_ic.set(Vec3(p.x, target + 1, p.z))){ ++target; }
				s.bots(i).smove(Vec3(0, std::min(target - p.y, MAX_LONG_DISTANCE), 0));
			}
		}
		s.commit();
	}
}

void collect_nanobots_z(State& s){
	while(true){
		bool done_flag = true;
		for(int i = 0; i < s.num_bots(); ++i){
			const auto p = s.bots(i).pos();
			if(p.z > 0){ done_flag = false; }
		}
		if(done_flag){ break; }
		InterferenceChecker initial_ic(s);
		InterferenceChecker move_ic(s);
		for(int i = 0; i < s.num_bots(); ++i){
			const auto p = s.bots(i).pos();
			if(p.z == 0 && !initial_ic.test(Vec3(p.x, p.y, 1))){
				s.bots(i).fusion_p(Vec3(0, 0,  1));
			}else if(p.z == 1 && !initial_ic.test(Vec3(p.x, p.y, 0))){
				s.bots(i).fusion_s(Vec3(0, 0, -1));
			}else{
				int target = p.z;
				while(target > 0 && move_ic.set(Vec3(p.x, p.y, target - 1))){ --target; }
				s.bots(i).smove(Vec3(0, 0, std::max(target - p.z, -MAX_LONG_DISTANCE)));
			}
		}
		s.commit();
	}
}

#endif
