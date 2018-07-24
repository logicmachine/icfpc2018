#include <queue>
#include <random>
#include <unordered_set>
#include <cassert>

#define STRONG_VALIDATION
#include "../../nbtasm/nbtasm.hpp"

#include "../libnbt/constants.hpp"
#include "../libnbt/vec2.hpp"
#include "../libnbt/array2d.hpp"
#include "../libnbt/array3d.hpp"
#include "../libnbt/planer_routing.hpp"
#include "../libnbt/collect_nanobots.hpp"

using namespace std;

static const int dx[] = { 1, 0, -1, 0 };
static const int dy[] = { 0, -1, 0, 1 };

static const int dx3[] = { 1, -1, 0, 0, 0, 0 };
static const int dy3[] = { 0, 0, 1, -1, 0, 0 };
static const int dz3[] = { 0, 0, 0, 0, 1, -1 };

static default_random_engine engine;

Array2D<uint8_t> extract_slice(const VoxelGrid& g, int y){
	const int n = g.size();
	Array2D<uint8_t> result(n, n);
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < n; ++j){ result(i, j) = g(i, y, j); }
	}
	return result;
}

Array2D<uint8_t> detect_ungrounded_areas(
	Array2D<uint8_t> cur_map,
	const Array2D<uint8_t>& prev_map,
	const Vec2& partition_size)
{
	const int n = cur_map.rows();
	const Vec2 n2(n, n);
	queue<Vec2> q;
	for(int z = 0; z < n; ++z){
		for(int x = 0; x < n; ++x){
			if(prev_map(z, x) && cur_map(z, x)){
				q.emplace(x, z);
				cur_map(z, x) = 0;
			}
		}
	}
	while(!q.empty()){
		const auto u = q.front();
		q.pop();
		for(int d = 0; d < 4; ++d){
			const auto v = u + Vec2(dx[d], dy[d]);
			if(v.x / partition_size.x != u.x / partition_size.x){ continue; }
			if(v.y / partition_size.y != u.y / partition_size.y){ continue; }
			if(v.region_check(n2) && cur_map(v)){
				q.push(v);
				cur_map(v) = 0;
			}
		}
	}
	return cur_map;
}

vector<Vec2> get_connected_components(
	Array2D<uint8_t> bitmap,
	const Vec2& root,
	const Vec2& partition_size)
{
	const Vec2 n2(bitmap.cols(), bitmap.rows());
	vector<Vec2> result;
	result.push_back(root);
	bitmap(root) = 0;
	for(size_t i = 0; i < result.size(); ++i){
		const auto u = result[i];
		for(int d = 0; d < 4; ++d){
			const auto v = u + Vec2(dx[d], dy[d]);
			if(v.x / partition_size.x != u.x / partition_size.x){ continue; }
			if(v.y / partition_size.y != u.y / partition_size.y){ continue; }
			if(v.region_check(n2) && bitmap(v)){
				result.push_back(v);
				bitmap(v) = 0;
			}
		}
	}
	return result;
}

void append_bone(
	Array3D<uint8_t>& bone, const Array3D<uint8_t>& model, int y, vector<Vec2> area)
{
	const int n = bone.depth();
	const Vec3 unreached(-1, -1, -1);
	Array3D<Vec3> prev(n, n, n, unreached);
	queue<Vec3> q;
	for(const auto v2 : area){
		const Vec3 v(v2.x, y, v2.y);
		q.push(v);
		prev(v) = v;
	}
	while(!q.empty()){
		const auto u = q.front();
		if(u.y == 0 || bone(u)){ break; }
		q.pop();
		for(int d = 0; d < 6; ++d){
			const Vec3 dv(dx3[d], dy3[d], dz3[d]);
			const auto v = u + dv;
			if(v.region_check(n) && model(v) && prev(v) == unreached){
				q.push(v);
				prev(v) = u;
			}
		}
	}
	assert(!q.empty());
	Vec3 cur = q.front();
	bone(cur) = 1;
	while(prev(cur) != cur){
		cur = prev(cur);
		bone(cur) = 1;
	}
}

inline int mdist(const Vec2& a, const Vec2& b){
	const auto c = a - b;
	return abs(c.x) + abs(c.y);
}

inline int mdist(const Vec3& a, const Vec3& b){
	const auto c = a - b;
	return abs(c.x) + abs(c.y) + abs(c.z);
}

template <typename T> inline int cols(const Array3D<T>& a){ return a.cols(); }
template <typename T> inline int rows(const Array3D<T>& a){ return a.rows(); }
template <typename T> inline int depth(const Array3D<T>& a){ return a.depth(); }
inline int cols(const VoxelGrid& a){ return a.size(); }
inline int rows(const VoxelGrid& a){ return a.size(); }
inline int depth(const VoxelGrid& a){ return a.size(); }

template <typename T>
vector<Vec3> bot_moveto(const T& matrix, const Vec3& init, const Vec3& target){
	unordered_map<Vec3, int> dist;
	unordered_map<Vec3, Vec3> history;
	priority_queue<pair<int, Vec3>, vector<pair<int, Vec3>>, greater<pair<int, Vec3>>> pq;
	dist[init] = 0;
	pq.emplace(mdist(init, target), init);
	while(!pq.empty()){
		const auto top = pq.top();
		if(top.second == target){ break; }
		pq.pop();
		const auto u = top.second;
		const int cur_h = mdist(u, target);
		const int cur_d = dist[top.second];
		if(top.first - cur_h > cur_d){ continue; }
		for(int d = 0; d < 6; ++d){
			for(int len = 1; len <= MAX_LONG_DISTANCE; ++len){
				const auto dv = Vec3(dx3[d], dy3[d], dz3[d]) * len;
				const auto v = u + dv;
				if(v.x < 0 || cols(matrix)  <= v.x){ break; }
				if(v.y < 0 || rows(matrix)  <= v.y){ break; }
				if(v.z < 0 || depth(matrix) <= v.z){ break; }
				if(matrix(v.z, v.y, v.x)){ break; }
				if(history.find(v) == history.end()){
					dist[v] = cur_d + 1;
					history[v] = dv;
					pq.emplace(cur_d + 1 + mdist(v, target), v);
				}else if(cur_d + 1 < dist[v]){
					dist[v] = cur_d + 1;
					history[v] = dv;
					pq.emplace(cur_d + 1 + mdist(v, target), v);
				}
			}
		}
	}
	if(pq.empty()){ return {}; }
	Vec3 cur = target;
	vector<Vec3> trace;
	while(cur != init){
		const auto dv = history[cur];
		trace.push_back(dv);
		cur -= dv;
	}
	reverse(trace.begin(), trace.end());
	return trace;
}

vector<Vec3> bot0_moveto(State& s, const Vec3& target){
	return bot_moveto(s.matrix(), s.bots(0).pos(), target);
}

void construct_bone(State& s, const Array3D<uint8_t>& bone, const Vec3& u){
	bool is_first = true;
	for(int d = 0; d < 6; ++d){
		const Vec3 dv(dx3[d], dy3[d], dz3[d]);
		const auto v = u + dv;
		if(!bone(v) || s.matrix(v.z, v.y, v.x)){ continue; }
		if(is_first){
			is_first = false;
			s.bots(0).smove(dv);
			s.commit();
			s.bots(0).fill(-dv);
			s.commit();
			construct_bone(s, bone, v);
		}else{
			const auto trace = bot0_moveto(s, v);
			for(const auto t : trace){
				s.bots(0).smove(t);
				s.commit();
			}
			construct_bone(s, bone, v);
		}
	}
	if(is_first){
		for(int d = 0; d < 6; ++d){
			const Vec3 dv(dx3[d], dy3[d], dz3[d]);
			const auto v = u + dv;
			if(s.matrix(v.z, v.y, v.x)){ continue; }
			s.bots(0).smove(dv);
			s.commit();
			s.bots(0).fill(-dv);
			s.commit();
			break;
		}
	}
}

Vec2 random_move_2d(Array2D<uint8_t>& obstacles, const Vec2& p){
	static const array<Vec2, 4> table = { Vec2(1, 0), Vec2(0, 1), Vec2(-1, 0), Vec2(0, -1) };
	const Vec2 area(obstacles.cols(), obstacles.rows());
	uniform_int_distribution<int> dir_dist(0, 3);
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

bool test_fillable(const Array3D<uint8_t>& matrix, const Vec3 p){
	if(p.x < 0 || matrix.cols()  <= p.x){ return false; }
	if(p.y < 0 || matrix.rows()  <= p.y){ return false; }
	if(p.z < 0 || matrix.depth() <= p.z){ return false; }
	if(matrix(p)){ return false; }
	const std::array<Vec3, 6> neighbors = {
		Vec3( 0, -1,  0),
		Vec3( 1,  0,  0),
		Vec3( 0,  0,  1),
		Vec3(-1,  0,  0),
		Vec3( 0,  0, -1),
		Vec3( 0,  1,  0),
	};
	for(const auto d : neighbors){
		const auto q = p + d;
		if(q.y < 0){ return true; }
		if(q.x < 0 || matrix.cols()  <= q.x){ continue; }
		if(q.y < 0 || matrix.rows()  <= q.y){ continue; }
		if(q.z < 0 || matrix.depth() <= q.z){ continue; }
		if(matrix(q)){ return true; }
	}
	return false;
}

vector<pair<Vec3, bool>> build_block_command_sequence(
	const Array3D<uint8_t>& bone,
	const Array3D<uint8_t>& model,
	const Vec2& partition_origin,
	const Vec2& partition_size,
	const Vec3& start_position)
{
	const int sub_w = min<int>(partition_size.x, model.cols()  - partition_origin.x);
	const int sub_h = min<int>(partition_size.y, model.depth() - partition_origin.y);
	const int height = model.rows();
	vector<pair<Vec3, bool>> sequence;

	// extract sub-block
	Array3D<uint8_t> sub_model(sub_h, height, sub_w);
	Array3D<uint8_t> sub_matrix(sub_h, height, sub_w);
	for(int z = 0; z < sub_h; ++z){
		for(int y = 0; y < height; ++y){
			for(int x = 0; x < sub_w; ++x){
				sub_model(z, y, x) = model(z + partition_origin.y, y, x + partition_origin.x);
				sub_matrix(z, y, x) = bone(z + partition_origin.y, y, x + partition_origin.x);
			}
		}
	}
	const Array3D<uint8_t> sub_bone = sub_matrix;

	const Vec3 local_init(
		start_position.x - partition_origin.x,
		start_position.y,
		start_position.z - partition_origin.y);
	Vec3 cur = local_init;

	for(int y = 1; y < height; ++y){
		while(true){
			bool has_update = true;
			// fill plane
			for(int iter = 0; iter % 2 != 0 || has_update; ++iter){
				if(iter % 2 == 0){ has_update = false; }
				for(int zz = 0; zz < sub_h; ++zz){
					const int z = ((iter & 1) == 0) ? zz : (sub_h - 1 - zz);
					for(int xx = 0; xx < sub_w; ++xx){
						const int x = ((z + (iter & 1)) % 2 == 0) ? xx : (sub_w - xx - 1);
						const Vec3 target(x, y, z);
						if(sub_matrix(target)){ continue; }
						const array<Vec3, 5> fillable = {
							Vec3( 0, -1,  0),
							Vec3( 1, -1,  0),
							Vec3( 0, -1,  1),
							Vec3(-1, -1,  0),
							Vec3( 0, -1, -1)
						};
						bool is_target = false;
						for(const auto nd : fillable){
							const auto p = target + nd;
							if(test_fillable(sub_matrix, p) && sub_model(p)){
								is_target = true;
							}
						}
						if(!is_target){ continue; }
						const auto trace = bot_moveto(sub_matrix, cur, target);
						for(const auto& t : trace){
							cur += t;
							sequence.emplace_back(t, false);
						}
						for(const auto nd : fillable){
							const auto p = target + nd;
							if(test_fillable(sub_matrix, p) && sub_model(p)){
								sequence.emplace_back(nd, true);
								sub_matrix(p) = true;
								has_update = true;
							}
						}
					}
				}
			}
			// check completion
			vector<Vec3> failed_voxels;
			for(int z = 0; z < sub_h; ++z){
				for(int x = 0; x < sub_w; ++x){
					const Vec3 p(z, y - 1, x);
					if(sub_model(p) && !sub_matrix(p)){
						failed_voxels.push_back(p);
					}
				}
			}
			if(failed_voxels.empty()){ break; }
			// revert
			const int failed_y = failed_voxels.front().y;
			cerr << "  revert: y = " << failed_y << ", n_failed = " << failed_voxels.size() << endl;
			while(!sequence.empty()){
				bool revert_completed = true;
				for(int z = 0; z < sub_h; ++z){
					for(int x = 0; x < sub_w; ++x){
						if(sub_bone(z, failed_y, x)){ continue; }
						if(sub_matrix(z, failed_y, x)){ revert_completed = false; }
					}
				}
				if(revert_completed){ break; }
				const auto c = sequence.back();
				sequence.pop_back();
				if(c.second){
					sub_matrix(cur + c.first) = false;
				}else{
					cur -= c.first;
				}
			}
			// filter failed voxels
			vector<Vec3> filtered_failed_voxels;
			for(const auto v : failed_voxels){
				if(test_fillable(sub_matrix, v)){ 
					filtered_failed_voxels.push_back(v);
				}
			}
			// fill failed voxel from side
			static const array<Vec3, 8> neighbors = {
				Vec3(-1,  0, -1), Vec3( 0,  0, -1), Vec3( 1,  0, -1),
				Vec3(-1,  0,  0),                   Vec3( 1,  0,  0),
				Vec3(-1,  0,  1), Vec3( 0,  0,  1), Vec3( 1,  0,  1),
			};
			shuffle(filtered_failed_voxels.begin(), filtered_failed_voxels.end(), engine);
			for(const auto failed_voxel : filtered_failed_voxels){
				for(const auto nd : neighbors){
					const auto target = failed_voxel + nd;
					const auto trace = bot_moveto(sub_matrix, cur, target);
					if(trace.empty() && cur != target){ continue; }
					for(const auto& t : trace){
						cur += t;
						sequence.emplace_back(t, false);
					}
					sequence.emplace_back(-nd, true);
					sub_matrix(target - nd) = true;
					break;
				}
			}
			for(const auto v : failed_voxels){
				assert(sub_matrix(v));
			}
		}
	}
	const auto trace = bot_moveto(sub_matrix, cur, local_init);
	for(const auto& t : trace){
		cur += t;
		sequence.emplace_back(t, false);
	}
	return sequence;
}


int main(int argc, char *argv[]){
	const auto model = VoxelGrid::load_model(argv[1]);

	State s(model.size(), 40);

	const int n = model.size();
	const Vec2 n2(n, n);
	const Vec2 partition_size(8, 8);

	Array3D<uint8_t> a3model(model.size(), model.size(), model.size());
	int ceil_y = 0;
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < n; ++j){
			for(int k = 0; k < n; ++k){
				a3model(i, j, k) = model(i, j, k);
				if(model(i, j, k)){ ceil_y = max(ceil_y, j + 1); }
			}
		}
	}

	// Build bones
	Array3D<uint8_t> bone(n, n, n);
	for(int y = 1; y < n; ++y){
		auto ungrounded_map = detect_ungrounded_areas(
			extract_slice(model, y), extract_slice(model, y - 1), partition_size);
		for(int z = 0; z < n; ++z){
			for(int x = 0; x < n; ++x){
				if(!ungrounded_map(z, x)){ continue; }
				const auto cc = get_connected_components(
					ungrounded_map, Vec2(x, z), partition_size);
				for(const auto v : cc){ ungrounded_map(v) = 0; }
				append_bone(bone, a3model, y, cc);
			}
		}
	}

	int bone_size = 0;
	for(int i = 0; i < n; ++i){
		for(int j = 0; j < n; ++j){
			for(int k = 0; k < n; ++k){
				bone_size += bone(i, j, k);
			}
		}
	}
	cerr << "Bone size: " << bone_size << endl;

	for(int z = 0; z < n; ++z){
		for(int x = 0; x < n; ++x){
			if(bone(z, 0, x) && !s.matrix(z, 0, x)){
				const auto trace = bot0_moveto(s, Vec3(x, 0, z));
				for(const auto t : trace){
					s.bots(0).smove(t);
					s.commit();
				}
				construct_bone(s, bone, Vec3(x, 0, z));
			}
		}
	}

	// enumerate partitions
	unordered_set<Vec2> partitions;
	for(int z0 = 0; z0 < n; z0 += partition_size.y){
		for(int x0 = 0; x0 < n; x0 += partition_size.x){
			const int x1 = min(n - 1, x0 + partition_size.x - 1);
			const int z1 = min(n - 1, z0 + partition_size.y - 1);
			if(model.test(Vec3(x0, 0, z0), Vec3(x1, n - 1, z1))){
				partitions.emplace(x0, z0);
			}
		}
	}

	{	// move bot0 to (n/2, ceil_y, n/2)
		const auto trace = bot0_moveto(s, Vec3(n / 2, ceil_y, n / 2));
		for(const auto t : trace){
			s.bots(0).smove(t);
			s.commit();
		}
	}

	{	// fork nanobots
		while(s.num_bots() < s.max_num_bots()){
			Array2D<uint8_t> obstacles(n, n);
			for(int i = 0; i < s.num_bots(); ++i){
				const auto p = s.bots(i).pos();
				obstacles(p.z, p.x) = true;
			}

			{	// fission from bots(0)
				const auto p = s.bots(0).pos();
				const Vec2 p2(p.x, p.z);
				for(int d = 0; d < 4; ++d){
					const Vec2 dv(dx[d], dy[d]);
					if(!obstacles(p2 + dv)){
						s.bots(0).fission(Vec3(dv.x, 0, dv.y), 0);
						obstacles(p2 + dv) = true;
						break;
					}
				}
			}
			// random walk
			for(int i = 1; i < s.num_bots(); ++i){
				const auto p3 = s.bots(i).pos();
				const Vec2 p2 = Vec2(p3.x, p3.z);
				const Vec2 d = random_move_2d(obstacles, p2);
				s.bots(i).smove(Vec3(d.x, 0, d.y));
			}
			s.commit();
		}
	}

	const int nbots = s.num_bots();
	vector<vector<pair<Vec3, bool>>> queues(nbots);
	while(true){
		Array2D<uint8_t> ceil_obstacles(n, n);

		bool done_all = partitions.empty();
		for(int i = 0; i < nbots; ++i){
			const auto p3 = s.bots(i).pos();
			if(!queues[i].empty()){ done_all = false; }
			if(p3.y == ceil_y){ ceil_obstacles(p3.z, p3.x) = true; }
		}
		if(done_all){ break; }

		// assign new blocks
		for(int i = 0; i < nbots; ++i){
			const auto p3 = s.bots(i).pos();
			if(!queues[i].empty() || p3.y != ceil_y){ continue; }
			const Vec2 p2(p3.x, p3.z);
			if(partitions.find(p2) != partitions.end()){
				cerr << "compute partition " << p2 << " => " << s.bots(i).bid() << endl;
				queues[i] = build_block_command_sequence(
					bone, a3model, p2, partition_size, p3);
				cerr << "  sequence length = " << queues[i].size() << endl;
				reverse(queues[i].begin(), queues[i].end());
				partitions.erase(p2);
			}
		}

		// process queued commands
		vector<bool> queue_pop(nbots, false);
		for(int i = 0; i < nbots; ++i){
			if(queues[i].empty()){ continue; }
			const auto p3 = s.bots(i).pos();
			const auto c = queues[i].back();
			if(c.second){
				s.bots(i).fill(c.first);
				queue_pop[i] = true;
			}else{
				const auto d = c.first;
				bool valid = true;
				if(p3.y == ceil_y){
					if(d.x < 0){
						for(int i = -1; i >= d.x; --i){
							if(ceil_obstacles(p3.z, p3.x + i)){ valid = false; break; }
							ceil_obstacles(p3.z, p3.x + i) = 1;
						}
					}else if(d.x > 0){
						for(int i = 1; i <= d.x; ++i){
							if(ceil_obstacles(p3.z, p3.x + i)){ valid = false; break; }
							ceil_obstacles(p3.z, p3.x + i) = 1;
						}
					}else if(d.z < 0){
						for(int i = -1; i >= d.z; --i){
							if(ceil_obstacles(p3.z + i, p3.x)){ valid = false; break; }
							ceil_obstacles(p3.z + i, p3.x) = 1;
						}
					}else if(d.z > 0){
						for(int i = 1; i <= d.z; ++i){
							if(ceil_obstacles(p3.z + i, p3.x)){ valid = false; break; }
							ceil_obstacles(p3.z + i, p3.x) = 1;
						}
					}
				}
				if(p3.y + d.y == ceil_y){
					ceil_obstacles(p3.z, p3.x) = 1;
				}
				if(valid){ 
					queue_pop[i] = valid;
					s.bots(i).smove(d);
				}
			}
		}

		// search new blocks
		vector<Vec2> sources;
		vector<int> source_indices;
		for(int i = 0; i < nbots; ++i){
			const auto p3 = s.bots(i).pos();
			if(!queues[i].empty()){ continue; }
			if(p3.y == ceil_y){
				sources.emplace_back(p3.x, p3.z);
				source_indices.push_back(i);
			}else{
				const Vec3 d(0, min(MAX_LONG_DISTANCE, ceil_y - p3.y), 0);
				if(!ceil_obstacles(p3.z, p3.x)){ s.bots(i).smove(d); }
				if(p3.y + d.y == ceil_y){ ceil_obstacles(p3.z, p3.x) = true; }
			}
		}
		const vector<Vec2> targets(partitions.begin(), partitions.end());
		const auto raw_moves = planer_routing(ceil_obstacles, sources, targets);
		for(int i = 0; i < source_indices.size(); ++i){
			const int index = source_indices[i];
			if(raw_moves[i] != Vec2()){
				// move to block origin
				const auto p2 = raw_moves[i];
				s.bots(index).smove(Vec3(p2.x, 0, p2.y));
			}else{
				// random walk
				const auto p3 = s.bots(index).pos();
				const Vec2 p2 = Vec2(p3.x, p3.z);
				const Vec2 d = random_move_2d(ceil_obstacles, p2);
				s.bots(index).smove(Vec3(d.x, 0, d.y));
			}
		}

		// update command queue
		for(int i = 0; i < nbots; ++i){
			if(queues[i].empty()){ continue; }
			if(queue_pop[i]){ queues[i].pop_back(); }
		}

		s.commit();
	}

	collect_nanobots_y_inv(s);
	collect_nanobots_x(s);
	collect_nanobots_z(s);
	collect_nanobots_y(s);

	s.bots(0).halt();
	s.dump_pending_commands(cerr);
	s.commit();
	cerr << "Energy: " << s.energy() << endl;

	s.export_trace(argv[2]);

	return 0;
}

