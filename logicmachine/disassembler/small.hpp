#ifndef DISASSEMBLER_SMALL_HPP
#define DISASSEMBLER_SMALL_HPP

#include "../../nbtasm/nbtasm.hpp"
#include "../libnbt/constants.hpp"
#include "../libnbt/collect_nanobots.hpp"
#include "../libnbt/no_conflict.hpp"

using namespace std;

class SmallSolver {

private:
	int vec2bits(const Vec3& v) const {
		return (v.x > 0 ? 1 : 0) | (v.y > 0 ? 2 : 0) | (v.z > 0 ? 4 : 0);
	}
	Vec3 bits2vec(int b) const {
		return Vec3((b & 1) ? 1 : 0, (b & 2) ? 1 : 0, (b & 4) ? 1 : 0);
	}

public:
	static bool is_solvable(const State& s){
		const size_t n = s.matrix().size();
		return n <= MAX_FAR_DISTANCE + 2;
	}

	void operator()(State& s){
		auto b2v = [](int b){ return Vec3(b & 1, (b >> 1) & 1, (b >> 2) & 1); };
		const int n = s.matrix().size();

		int x_max = 0, y_max = 0, z_max = 0;
		for(int z = 0; z < n; ++z){
			for(int y = 0; y < n; ++y){
				for(int x = 0; x < n; ++x){
					if(!s.matrix(z, y, x)){ continue; }
					x_max = max(x_max, x);
					y_max = max(y_max, y);
					z_max = max(z_max, z);
				}
			}
		}
		++x_max;
		++z_max;

		vector<int> to_fission = {
			7, // ()   -> (x, y, z)
			2, // (x)  -> (xy)
			4, // (y)  -> (yz)
			4, // (xy) -> (xyz)
			1, // (z)  -> (xz)
			0,
			0,
			0
		};

		while(true){
			bool done_flag = true;
			for(int i = 0; i < s.num_bots(); ++i){
				const auto p = s.bots(i).pos();
				if(p.x > 0 && p.x < x_max){
					dispatch_move_no_conflict(s, i, Vec3(x_max, p.y, p.z));
					done_flag = false;
				}else if(p.y > 0 && p.y < y_max){
					dispatch_move_no_conflict(s, i, Vec3(p.x, y_max, p.z));
					done_flag = false;
				}else if(p.z > 0 && p.z < z_max){
					dispatch_move_no_conflict(s, i, Vec3(p.x, p.y, z_max));
					done_flag = false;
				}else{
					const int id = vec2bits(s.bots(i).pos());
					if(to_fission[id] == 0){ continue; }
					const int to = __builtin_ctz(to_fission[id]);
					const int m = __builtin_popcountll(s.bots(i).seeds());
					to_fission[id] ^= (1 << to);
					s.bots(i).fission(bits2vec(1 << to), m / 2);
					done_flag = false;
				}
			}
			if(done_flag){ break; }
			s.commit();
		}

		for(int i = 0; i < 8; ++i){
			const int fsize = n - 3;
			const auto p = s.bots(i).pos();
			const Vec3 nd(p.x ? -1 : 1, 0, p.z ? -1 : 1);
			const Vec3 fd(
				(p.x ? -1 : 1) * (x_max - 2),
				(p.y ? -1 : 1) * y_max,
				(p.z ? -1 : 1) * (z_max - 2));
			s.bots(i).gempty(nd, fd);
		}
		s.commit();

		collect_nanobots_x(s);
		collect_nanobots_y(s);
		collect_nanobots_z(s);

		s.bots(0).halt();
		s.commit();
	}

};

#endif
