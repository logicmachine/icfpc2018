#ifndef DISASSEMBLER_LARGE_HPP
#define DISASSEMBLER_LARGE_HPP

#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <random>
#include <cassert>

#include "../../nbtasm/nbtasm.hpp"
#include "../libnbt/constants.hpp"
#include "../libnbt/no_conflict.hpp"
#include "../libnbt/array2d.hpp"
#include "../libnbt/planer_routing.hpp"
#include "../libnbt/collect_nanobots.hpp"

using namespace std;

class LargeSolver {

private:
	default_random_engine engine;

	Box compute_bounding_box(const VoxelGrid& g) const {
		const int n = g.size();
		Vec3 minval(n, n, n), maxval(0, 0, 0);
		for(int z = 0; z < n; ++z){
			for(int y = 0; y < n; ++y){
				for(int x = 0; x < n; ++x){
					if(!g(z, y, x)){ continue; }
					minval.x = min(minval.x, x);
					maxval.x = max(maxval.x, x);
					minval.y = min(minval.y, y);
					maxval.y = max(maxval.y, y);
					minval.z = min(minval.z, z);
					maxval.z = max(maxval.z, z);
				}
			}
		}
		return Box(minval, maxval);
	}

	vector<int> enumerate_hole_points(int minval, int maxval) const {
		vector<int> ret;
		for(int i = minval - 1; i < maxval + 1; i += MAX_FAR_DISTANCE){ ret.push_back(i); }
		ret.push_back(maxval + 1);
		return ret;
	}

	Vec2 random_move_2d(Array2D<uint8_t>& obstacles, const Vec2& p){
		static const array<Vec2, 4> table = { Vec2(1, 0), Vec2(0, 1), Vec2(-1, 0), Vec2(0, -1) };
		const Vec2 area(obstacles.cols(), obstacles.rows());
		uniform_int_distribution<int> dir_dist(0, 4);
		uniform_int_distribution<int> len_dist(0, MAX_SHORT_DISTANCE);
		Vec2 d;
		int len;
		while(true){
			d = table[dir_dist(engine)];
			len = len_dist(engine);
			bool accept = true;
			for(int i = 1; i <= len; ++i){
				if(!(p + d * i).region_check(area)){ accept = false; }
				if(obstacles(p + d * i)){ accept = false; }
			}
			if(accept){ break; }
		}
		for(int i = 0; i <= len; ++i){
			obstacles(p + d * i) = 1;
		}
		return d * len;
	}


public:
	static bool is_solvable(const State& s){
		return true;
	}

	void operator()(State& s){
		const auto size = s.matrix().size();
		const auto bounding_box = compute_bounding_box(s.matrix());
		const auto hole_xs = enumerate_hole_points(bounding_box.p.x, bounding_box.q.x);
		const auto hole_zs = enumerate_hole_points(bounding_box.p.z, bounding_box.q.z);
		const int ceil_y = bounding_box.q.y + 1;
		const int target_num_bots = min<int>(s.max_num_bots(), 2 * hole_xs.size() * hole_zs.size());

		{	// move to (hole_xs[0], ceil, hole_zs[0])
			const Vec3 t(hole_xs[0], ceil_y, hole_zs[0]);
			while(!dispatch_move_no_conflict(s, 0, t)){ s.commit(); }
		}
		{	// dig holes
			while(true){
				vector<Vec2> holes;
				for(const auto z : hole_zs){
					for(const auto x : hole_xs){
						bool empty = true;
						for(int y = 0; empty && y < ceil_y; ++y){
							if(s.matrix(z, y, x)){ empty = false; }
						}
						if(!empty){ holes.emplace_back(x, z); }
					}
				}
				if(holes.empty()){ break; }
				const int n_holes = holes.size();

				Array2D<uint8_t> obstacles(size, size, false);
				for(int i = 0; i < s.num_bots(); ++i){
					const auto p = s.bots(i).pos();
					obstacles(p.z, p.x) = true;
				}

				bool do_fission = (s.num_bots() < target_num_bots);
				if(do_fission){
					const auto p = s.bots(0).pos();
					const Vec2 p2(p.x, p.z);
					for(const auto d : { Vec2(1, 0), Vec2(0, 1), Vec2(1, 1) }){
						if(!obstacles(p2 + d)){
							s.bots(0).fission(Vec3(d.x, 0, d.y), 0);
							obstacles(p2 + d) = true;
							break;
						}
					}
				}

				vector<Vec2> sources;
				vector<int> sources_index;
				for(int i = 0; i < s.num_bots(); ++i){
					const auto p = s.bots(i).pos();
					if(p.y == ceil_y){
						sources.emplace_back(p.x, p.z);
						sources_index.push_back(i);
					}
				}
				const auto raw_moves = planer_routing(obstacles, sources, holes);
				vector<Vec2> moves(s.num_bots());
				for(int i = 0; i < sources_index.size(); ++i){
					moves[sources_index[i]] = raw_moves[i];
				}

				const unordered_set<Vec2> holes_set(holes.begin(), holes.end());
				for(int i = 0; i < s.num_bots(); ++i){
					if(do_fission && i == 0){ continue; }
					const auto p3 = s.bots(i).pos();
					const Vec2 p2 = Vec2(p3.x, p3.z);
					if(moves[i] == Vec2()){
						if(holes_set.find(p2) == holes_set.end()){
							if(p3.y < ceil_y){
								s.bots(i).smove(Vec3(0, min(MAX_LONG_DISTANCE, ceil_y - p3.y), 0));
							}else{
								const Vec2 d = random_move_2d(obstacles, p2);
								s.bots(i).smove(Vec3(d.x, 0, d.y));
							}
						}else{
							int target_y = p3.y;
							while(!s.matrix(p3.z, target_y, p3.x)){ --target_y; }
							if(target_y == p3.y - 1){
								s.bots(i).empty(Vec3(0, -1, 0));
							}else{
								s.bots(i).smove(Vec3(0, max(-MAX_LONG_DISTANCE, (target_y + 1) - p3.y), 0));
							}
						}
					}else{
						s.bots(i).smove(Vec3(moves[i].x, 0, moves[i].y));
					}
				}
				s.commit();
			}
		}
		{	// prepare nanobots (fission, smove(0, y, 0))
			while(true){
				const bool fission_done = (s.num_bots() == target_num_bots);

				Array2D<uint8_t> obstacles(size, size, false);
				bool move_done = true;
				for(int i = 0; i < s.num_bots(); ++i){
					const auto p = s.bots(i).pos();
					if(p.y != ceil_y){ move_done = false; }
					obstacles(p.z, p.x) = true;
				}
				if(fission_done && move_done){ break; }

				if(!fission_done){
					const auto p = s.bots(0).pos();
					const Vec2 p2(p.x, p.z);
					for(const auto d : { Vec2(1, 0), Vec2(0, 1), Vec2(1, 1) }){
						if(!obstacles(p2 + d)){
							s.bots(0).fission(Vec3(d.x, 0, d.y), 0);
							obstacles(p2 + d) = true;
							break;
						}
					}
				}

				for(int i = 1; i < s.num_bots(); ++i){
					const auto p3 = s.bots(i).pos();
					if(p3.y < ceil_y){
						s.bots(i).smove(Vec3(0, min(MAX_LONG_DISTANCE, ceil_y - p3.y), 0));
					}else{
						const Vec2 p2(p3.x, p3.z);
						const Vec2 d = random_move_2d(obstacles, p2);
						s.bots(i).smove(Vec3(d.x, 0, d.y));
					}
				}
				s.commit();
			}
		}
		// compute target rectangles
		using tiiii = tuple<int, int, int, int>;
		vector<tiiii> target_rectangles;
		if(hole_xs.size() * hole_zs.size() * 2 <= s.num_bots()){
			target_rectangles.emplace_back(0, 0, hole_xs.size() - 1, hole_zs.size() - 1);
		}else{
			for(int gz0 = 0; gz0 + 1 < hole_zs.size(); gz0 += 3){
				for(int gx0 = 0; gx0 + 1 < hole_xs.size(); gx0 += 4){
					const int gz1 = min<int>(hole_zs.size() - 1, gz0 + 3);
					const int gx1 = min<int>(hole_xs.size() - 1, gx0 + 4);
					target_rectangles.emplace_back(gx0, gz0, gx1, gz1);
				}
			}
		}
		// erase voxels
		for(const auto& target_rectangle : target_rectangles){
			const int xs_lo = get<0>(target_rectangle);
			const int ys_lo = get<1>(target_rectangle);
			const int xs_hi = get<2>(target_rectangle);
			const int ys_hi = get<3>(target_rectangle);

			vector<Vec2> holes;
			for(int x = xs_lo; x <= xs_hi; ++x){
				for(int y = ys_lo; y <= ys_hi; ++y){
					holes.emplace_back(hole_xs[x], hole_zs[y]);
				}
			}
			const unordered_set<Vec2> holes_set(holes.begin(), holes.end());

			vector<int> hole_row(s.num_bots(), -1);
			vector<int> hole_col(s.num_bots(), -1);
			vector<int> hole_depth(s.num_bots(), -1);
			for(int iter = 0; iter < 2; ++iter){ // move nanobots to holes
				while(true){
					Array2D<uint8_t> obstacles(size, size, false);
					vector<Vec2> sources;
					vector<int> source_indices;
					int done_count = 0;
					for(int i = 0; i < s.num_bots(); ++i){
						if(hole_depth[i] >= 0){ continue; }
						const auto p3 = s.bots(i).pos();
						const Vec2 p2(p3.x, p3.z);
						sources.emplace_back(p2);
						source_indices.push_back(i);
						obstacles(p2) = 1;
						if(holes_set.find(p2) != holes_set.end()){ ++done_count; }
					}
					if(done_count == holes.size()){ break; }
					const auto raw_moves = planer_routing(obstacles, sources, holes);
					vector<Vec2> moves(s.num_bots());
					for(int i = 0; i < sources.size(); ++i){
						moves[source_indices[i]] = raw_moves[i];
					}
					for(int i = 0; i < s.num_bots(); ++i){
						if(hole_depth[i] >= 0){ continue; }
						const auto p3 = s.bots(i).pos();
						const Vec2 p2(p3.x, p3.z);
						if(holes_set.find(p2) != holes_set.end()){ continue; }
						if(moves[i] == Vec2()){
							const Vec2 d = random_move_2d(obstacles, p2);
							s.bots(i).smove(Vec3(d.x, 0, d.y));
						}else{
							s.bots(i).smove(Vec3(moves[i].x, 0, moves[i].y));
						}
					}
					s.commit();
				}
				for(int i = 0; i < s.num_bots(); ++i){
					if(hole_depth[i] >= 0){ continue; }
					const auto p3 = s.bots(i).pos();
					const Vec2 p2(p3.x, p3.z);
					if(holes_set.find(p2) == holes_set.end()){ continue; }
					hole_row[i] = lower_bound(hole_zs.begin(), hole_zs.end(), p3.z) - hole_zs.begin();
					hole_col[i] = lower_bound(hole_xs.begin(), hole_xs.end(), p3.x) - hole_xs.begin();
					hole_depth[i] = iter;
				}
				if(iter == 0){ // lower nanobots
					while(true){
						bool done_flag = true;
						for(int i = 0; i < s.num_bots(); ++i){
							if(hole_depth[i] != 0){ continue; }
							const Vec3 p3 = s.bots(i).pos();
							const Vec3 t(p3.x, max(0, ceil_y - MAX_FAR_DISTANCE), p3.z);
							if(!dispatch_move_no_conflict(s, i, t)){ done_flag = false; }
						}
						if(done_flag){ break; }
						s.commit();
					}
				}
			}
			// erase voxels
			for(int h_hi = ceil_y; h_hi > 0; h_hi = max(0, h_hi - MAX_FAR_DISTANCE)){
				const int h[] = { max(0, h_hi - MAX_FAR_DISTANCE), h_hi };
				{	// test voxels
					const Vec3 p(hole_xs[xs_lo], 0, hole_zs[ys_lo]);
					const Vec3 q(hole_xs[xs_hi], h_hi, hole_zs[ys_hi]);
					if(!s.matrix().test(p, q)){ break; }
				}
				// move nanobots vertically
				for(int iter = 0; iter < 2; ++iter){
					while(true){
						bool done_flag = true;
						for(int i = 0; i < s.num_bots(); ++i){
							if(hole_depth[i] != iter){ continue; }
							const Vec3 p3 = s.bots(i).pos();
							const Vec3 t(p3.x, h[iter], p3.z);
							if(!dispatch_move_no_conflict(s, i, t)){ done_flag = false; }
						}
						if(done_flag){ break; }
						s.commit();
					}
				}
				// turn on harmonics
				if(s.harmonics() == Harmonics::Low){
					s.bots(0).flip();
					s.commit();
				}
				// call gempty
				for(int block = 0; block < 4; ++block){
					const int by = (block >> 1), bx = (block & 1);
					for(int i = 0; i < s.num_bots(); ++i){
						const Vec2 dtable[] = { Vec2(0, 0), Vec2(0, -1), Vec2(-1, -1), Vec2(-1, 0) };
						const Vec2 ndtable[] = { Vec2(1, 0), Vec2(0, -1), Vec2(-1, 0), Vec2(0, 1) };
						const int hy = hole_row[i], hx = hole_col[i];
						for(int di = 0; di < 4; ++di){
							const auto d = dtable[di];
							const int gy = hy + d.y, gx = hx + d.x;
							if(gy < ys_lo || ys_hi <= gy || gx < xs_lo || xs_hi <= gx){ continue; }
							if((gy & 1) != by || (gx & 1) != bx){ continue; }
							const Vec2 nd2 = ndtable[(di - ((hy ^ hx) & 1) + 4) % 4];
							const Vec3 nd3(nd2.x, hole_depth[i] == 0 ? 0 : -1, nd2.y);
							const int yy = h[1] - h[0] - 1;
							const int xx = hole_xs[gx + 1] - hole_xs[gx] - 2 * abs(nd3.x);
							const int zz = hole_zs[gy + 1] - hole_zs[gy] - 2 * abs(nd3.z);
							if(xx < 0 || zz < 0){ continue; }
							const Vec3 fd3(
								xx * (d.x >= 0 ? 1 : -1),
								yy * (hole_depth[i] == 0 ? 1 : -1),
								zz * (d.y >= 0 ? 1 : -1));
							const auto bp = s.bots(i).pos() + nd3;
							const auto bq = bp + fd3;
							if(yy == 0 && nd3.y < 0){ continue; }
							if(xx == 0 && nd3.x < 0){ continue; }
							if(zz == 0 && nd3.z < 0){ continue; }
							if(s.matrix().test(bp, bq)){
								s.bots(i).gempty(nd3, fd3);
							}
						}
					}
					// s.dump_pending_commands(cerr);
					s.commit();
				}
			}
			if(target_rectangle == target_rectangles.back()){
				break;
			}else{
				// back to plane y=ceil_y
				while(true){
					bool done_flag = true;
					for(int i = 0; i < s.num_bots(); ++i){
						if(s.bots(i).pos().y != ceil_y){ done_flag = false; }
					}
					if(done_flag){ break; }

					Array2D<uint8_t> obstacles(size, size, false);
					for(int i = 0; i < s.num_bots(); ++i){
						const auto p = s.bots(i).pos();
						obstacles(p.z, p.x) = true;
					}
					InterferenceChecker ic(s);
					for(int i = 0; i < s.num_bots(); ++i){
						const auto p = s.bots(i).pos();
						if(p.y == ceil_y){
							const Vec2 p2(p.x, p.z);
							const Vec2 d = random_move_2d(obstacles, p2);
							s.bots(i).smove(Vec3(d.x, 0, d.y));
						}else{
							int target = p.y;
							while(target < ceil_y && ic.set(Vec3(p.x, target + 1, p.z))){ ++target; }
							s.bots(i).smove(Vec3(0, min(target - p.y, MAX_LONG_DISTANCE), 0));
						}
					}
					s.commit();
				}
			}
		}
		// turn off harmonics
		if(s.harmonics() == Harmonics::High){
			s.bots(0).flip();
			s.commit();
		}

		// collect and halt
		collect_nanobots_y(s);
		collect_nanobots_x(s);
		collect_nanobots_z(s);

		s.bots(0).halt();
		s.commit();
	}

};

#endif
