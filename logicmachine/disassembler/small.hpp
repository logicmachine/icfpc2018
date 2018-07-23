#ifndef DISASSEMBLER_SMALL_HPP
#define DISASSEMBLER_SMALL_HPP

#include "../../nbtasm/nbtasm.hpp"
#include "../libnbt/constants.hpp"

using namespace std;

class SmallSolver {

public:
	static bool is_solvable(const State& s){
		const size_t n = s.matrix().size();
		return n <= MAX_FAR_DISTANCE + 2;
	}

	void operator()(State& s){
		auto b2v = [](int b){ return Vec3(b & 1, (b >> 1) & 1, (b >> 2) & 1); };
		const int n = s.matrix().size();

		s.bots(0).fission(Vec3(1, 0, 0), 2); // (0, 0, 0) -> (1, 0, 0)
		s.commit();

		s.bots(0).fission(Vec3(0, 1, 0), 1); // (0, 0, 0) -> (0, 1, 0)
		s.commit();

		s.bots(0).fission(Vec3(0, 0, 1), 1); // (0, 0, 0) -> (0, 0, 1)
		s.commit();

		for(int i = 1; i < n; i += MAX_LONG_DISTANCE){
			const int len = min(MAX_LONG_DISTANCE, n - 1 - i);
			s.bots(1).smove(Vec3(len, 0, 0)); // (0, 0, 0) -> (1, 0, 0)
			s.bots(2).smove(Vec3(0, len, 0)); // (0, 0, 0) -> (0, 1, 0)
			s.bots(3).smove(Vec3(0, 0, len)); // (0, 0, 0) -> (0, 0, 1)
			s.commit();
		}

		s.bots(1).fission(Vec3(0, 0, 1), 1); // (1, 0, 0) -> (1, 0, 1)
		s.bots(2).fission(Vec3(1, 0, 0), 0); // (0, 1, 0) -> (1, 1, 0)
		s.bots(3).fission(Vec3(0, 1, 0), 0); // (0, 0, 1) -> (0, 1, 1)
		s.commit();

		for(int i = 1; i < n; i += MAX_LONG_DISTANCE){
			const int len = min(MAX_LONG_DISTANCE, n - 1 - i);
			s.bots(2).smove(Vec3(0, 0, len)); // (1, 0, 0) -> (1, 0, 1)
			s.bots(4).smove(Vec3(len, 0, 0)); // (0, 1, 0) -> (1, 1, 0)
			s.bots(6).smove(Vec3(0, len, 0)); // (0, 0, 1) -> (0, 1, 1)
			s.commit();
		}

		s.bots(2).fission(Vec3(0, 1, 0), 0); // (1, 0, 1) -> (1, 1, 1)
		s.commit();

		for(int i = 1; i < n; i += MAX_LONG_DISTANCE){
			const int len = min(MAX_LONG_DISTANCE, n - 1 - i);
			s.bots(3).smove(Vec3(0, len, 0)); // (1, 0, 1) -> (1, 1, 1)
			s.commit();
		}

		for(int i = 0; i < 8; ++i){
			const int fsize = n - 3;
			const auto p = s.bots(i).pos();
			const Vec3 nd(p.x ? -1 : 1, 0, p.z ? -1 : 1);
			const Vec3 fd(nd.x * fsize, (p.y ? -1 : 1) * (n - 1), nd.z * fsize);
			s.bots(i).gempty(nd, fd);
		}
		s.commit();

		for(int i = 1; i < n; i += MAX_LONG_DISTANCE){
			const int len = min(MAX_LONG_DISTANCE, n - 1 - i);
			s.bots(3).smove(Vec3(0, -len, 0)); // (1, 1, 1) -> (1, 0, 1)
			s.commit();
		}

		s.bots(2).fusion_p(Vec3(0,  1, 0)); // (1, 1, 1) -> (1, 0, 1) -> (1, 1, 1)
		s.bots(3).fusion_s(Vec3(0, -1, 0));
		s.commit();

		for(int i = 1; i < n; i += MAX_LONG_DISTANCE){
			const int len = min(MAX_LONG_DISTANCE, n - 1 - i);
			s.bots(2).smove(Vec3(0, 0, -len)); // (1, 0, 1) -> (1, 0, 0)
			s.bots(4).smove(Vec3(-len, 0, 0)); // (1, 1, 0) -> (0, 1, 0)
			s.bots(6).smove(Vec3(0, -len, 0)); // (0, 1, 1) -> (0, 0, 1)
			s.commit();
		}

		s.bots(1).fusion_p(Vec3( 0,  0,  1)); // (1, 0, 1) -> (1, 0, 0)
		s.bots(2).fusion_s(Vec3( 0,  0, -1)); // (1, 0, 1) -> (1, 0, 0)
		s.bots(3).fusion_p(Vec3( 1,  0,  0)); // (1, 1, 0) -> (0, 1, 0)
		s.bots(4).fusion_s(Vec3(-1,  0,  0)); // (1, 1, 0) -> (0, 1, 0)
		s.bots(5).fusion_p(Vec3( 0,  1,  0)); // (0, 1, 1) -> (0, 0, 1)
		s.bots(6).fusion_s(Vec3( 0, -1,  0)); // (0, 1, 1) -> (0, 0, 1)
		s.commit();

		for(int i = 1; i < n; i += MAX_LONG_DISTANCE){
			const int len = min(MAX_LONG_DISTANCE, n - 1 - i);
			s.bots(1).smove(Vec3(-len, 0, 0)); // (1, 0, 0) -> (0, 0, 0)
			s.bots(2).smove(Vec3(0, -len, 0)); // (0, 1, 0) -> (0, 0, 0)
			s.bots(3).smove(Vec3(0, 0, -len)); // (0, 0, 1) -> (0, 0, 0)
			s.commit();
		}

		s.bots(0).fusion_p(Vec3( 1, 0, 0));
		s.bots(1).fusion_s(Vec3(-1, 0, 0));
		s.commit();

		s.bots(0).fusion_p(Vec3(0,  1, 0));
		s.bots(1).fusion_s(Vec3(0, -1, 0));
		s.commit();

		s.bots(0).fusion_p(Vec3(0, 0,  1));
		s.bots(1).fusion_s(Vec3(0, 0, -1));
		s.commit();

		s.bots(0).halt();
		s.commit();
	}

};

#endif
