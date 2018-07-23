#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <utility>
#include <queue>
#include <tuple>
#include <cstdint>
#include <cmath>

//#include "../nbtasm/nbtasm.hpp"
#include "nbtasm.hpp"

//#define DEBUG_PRINT

using namespace std;

constexpr int NBOTS = 40;
//constexpr int NROWS = 5;
//constexpr int NCOLS = 8;
constexpr int VOXELS_PER_BOT = 10;

typedef pair<string, vector<int>> command_t;

VoxelGrid read_data(string fname)
{
	fstream fs(fname.c_str(), ios_base::in | ios_base::binary);

	unsigned char x;
	fs.read((char*)&x, 1);
	int R = static_cast<unsigned int>(x);
	VoxelGrid v(R);
	int cnt = 0;
	for (int i = 0; i < (R * R * R - 1) / 8 + 1; i++) {
		fs.read((char*)&x, 1);
		for (int j = 0; j < 8; j++) {
			v(cnt % R, (cnt / R) % R, cnt / (R * R)) = static_cast<uint8_t>(x & 0x1);
			x >>= 1;
			cnt++;
		}
	}
	
	return v;
}

typedef pair<int, vector<Vec3>> score_t;

class AStar {
private:
	const State& state;
	const VoxelGrid& mat;
	Vec3 start, goal;
	vector<Vec3> volatile_map;

public:
	AStar(const State& state) : state(state), mat(state.matrix()) { }

	vector<Vec3> neighborhood(const Vec3& pos);
	vector<Vec3> search();

	void set(const Vec3& start_, const Vec3& goal_) {
		start = start_;
		goal = goal_;
	}
	int heuristic(const Vec3& pos) {
		int x = pos.x - goal.x;
		int y = pos.y - goal.y;
		int z = pos.z - goal.z;
		return abs(x) + abs(y) + abs(z);
	}
	int distance(const vector<Vec3>& path) {
		return path.size();
	}
	int set_volatile_map(int x, int y, int z) {
		volatile_map.push_back(Vec3{x, y, z});
	}
	int set_volatile_map(const Vec3& v) {
		volatile_map.push_back(v);
	}
	const vector<Vec3>& get_volatile_map() const {
		return volatile_map;
	}
	int clear_volatile_map() {
		volatile_map.clear();
	}
};

vector<Vec3> AStar::neighborhood(const Vec3& pos)
{
	int x = pos.x;
	int y = pos.y;
	int z = pos.z;
	vector<Vec3> neighbors;

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				if (abs(i) + abs(j) + abs(k) > 1 || (i == 0 && j == 0 && k == 0)) continue;
				if (z + k < 0 || z + k >= mat.r() || y + j < 0 || y + j >= mat.r() || x + i < 0 || x + i >= mat.r()) continue;
				if (find(volatile_map.begin(), volatile_map.end(), Vec3{x + i, y + j, z + k}) != volatile_map.end()) continue;
				if (mat(z + k, y + j, x + i) == 0)
					neighbors.push_back(Vec3{x + i, y + j, z + k});
			}
		}
	}

	return neighbors;
}

vector<Vec3> AStar::search()
{
	vector<Vec3> path;
	vector<Vec3> checked;
	checked.push_back(start);
	priority_queue<score_t, vector<score_t>, greater<score_t> > queue;
	queue.push(make_pair(distance(checked) + heuristic(start), checked));

	int cnt = 0;
	while (!queue.empty()) {
		score_t s = queue.top();
		queue.pop();
		int score = s.first;
		path = s.second;
		Vec3 last = *(path.end() - 1);
		if (last == goal) return path;
		vector<Vec3> neighbors = neighborhood(last);
		for (vector<Vec3>::iterator p = neighbors.begin(); p != neighbors.end(); ++p) {
			if (binary_search(checked.begin(), checked.end(), *p)) continue;
			checked.push_back(*p);
			sort(checked.begin(), checked.end());
			vector<Vec3> newpath(path);
			newpath.push_back(*p);
			queue.push(make_pair(distance(newpath) + heuristic(*p), newpath));
		}
		cnt++;
		if (cnt > 2000) break; ///// debug
		//if (queue.size() > 1000) break; ///// debug
	}

	return path;
}

vector<command_t> create_move_commands(const vector<Vec3>& path)
{
	vector<Vec3> diffs;
	vector<Vec3> vs;
	vector<command_t> cmds;

#ifdef DEBUG_PRINT
	cerr << "path: " << path.size() << " : "; /// debug
	for (const auto& pos : path) cerr << pos << "-"; /// debug
	cerr << endl; ///// debug
#endif
	if (path.size() <= 1) return cmds;

	for (int i = 0; i < static_cast<int>(path.size()) - 1; i++) {
		Vec3 diff = path[i+1] - path[i];
		diffs.push_back(diff);
	}
	Vec3 prev_diff(diffs[0]);
	Vec3 current_diff{0, 0, 0};
	for (int i = 1; i < static_cast<int>(diffs.size()); i++) {
		current_diff += prev_diff;
		if (diffs[i] != prev_diff) {
			vs.push_back(current_diff);
			current_diff = Vec3{0, 0, 0};
		}
		prev_diff = diffs[i];
	}
	current_diff += prev_diff;
	vs.push_back(current_diff);
	int idx = -1;
	int value = 0;
	int sign = 0;
	for (const Vec3& v : vs) {
		if (v.x != 0) {
			sign = v.x > 0 ? 1 : -1;
			idx = 0;
			value = abs(v.x);
			while (value > 0) {
				cmds.push_back(make_pair("smove", vector<int>{(value >= 15 ? 15 : value % 15) * sign, 0, 0}));
				value -= 15;
			}
		} else if (v.y != 0) {
			sign = v.y > 0 ? 1 : -1;
			idx = 1;
			value = abs(v.y);
			while (value > 0) {
				cmds.push_back(make_pair("smove", vector<int>{0, (value >= 15 ? 15 : value % 15) * sign, 0}));
				value -= 15;
			}
		} else if (v.z != 0) {
			sign = v.z > 0 ? 1 : -1;
			idx = 2;
			value = abs(v.z);
			while (value > 0) {
				cmds.push_back(make_pair("smove", vector<int>{0, 0, (value >= 15 ? 15 : value % 15) * sign}));
				value -= 15;
			}
		} else {
			cmds.push_back(make_pair("unknown", vector<int>{}));
		}
	}

	return cmds;
}

int execute(State& s, int idx, const vector<command_t>& cmds)
{
	for (const auto& cmd : cmds) {
		string c = cmd.first;
		vector<int> args = cmd.second;
		if (c == "smove") {
			//cerr << Vec3{args[0], args[1], args[2]} << endl; ///// debug
			s.bots(idx).smove(Vec3{args[0], args[1], args[2]});
		} else if (c == "fill") {
			s.bots(idx).fill(Vec3{args[0], args[1], args[2]});
		} else if (c == "flip") {
			s.bots(idx).flip();
		} else if (c == "wait") {
			s.bots(idx).wait();
		} else if (c == "halt") {
			s.bots(idx).halt();
		} else {
			;
		}
		s.commit();
	}
	
	return 0;
}

bool is_grounded(const VoxelGrid& mat, int x, int y, int z)
{
	if (y == 0) return true;

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			for (int k = -1; k <= 1; k++) {
				if (abs(i) + abs(j) + abs(k) > 1 || (i == 0 && j == 0 && k == 0)) continue;
				if (z + k < 0 || z + k >= mat.r() || y + j < 0 || y + j >= mat.r() || x + i < 0 || x + i >= mat.r()) continue;
				if (mat(z + k, y + j, x + i) == 1)
					return true;
			}
		}
	}
	
	return false;
}


class BotAgent {
private:
	State& s;

public:
	vector<command_t> cmds;
	int idx;

	BotAgent(State& s) : s(s) { }
};

class BotManager {
private:
	vector<BotAgent> agents;
	VoxelGrid& v;
	State& s;
	AStar& astar;
	int R;
	int current_agent;

public:
	BotManager(int n, VoxelGrid& v, State& s, AStar& astar) : agents(n, BotAgent(s)), v(v), s(s), astar(astar), R(v.r()) {
		current_agent = 0;
		for (int i = 0; i < n; i++) {
			agents[i].idx = i;
		}
	}

	void wave() {
		for (int i = 1; i < get_size(); i++) {
			int man_idx = i / 10;
			int home_y = i % 10;
			int home_x = 0;
			int home_z = 0;
			switch (man_idx) {
			case 0:
				home_x = 0;
				home_z = 0;
				break;
			case 1:
				home_x = R - 1;
				home_z = 0;
				break;
			case 2:
				home_x = 0;
				home_z = R - 1;
				break;
			case 3:
				home_x = R - 1;
				home_z = R - 1;
				break;
			default:
				break;
			}

			for (int j = 0; j < i; j++)
				astar.set_volatile_map(s.bots(j).pos());
			astar.set(Vec3{1, 0, 0}, Vec3{home_x, home_y, home_z});
			vector<Vec3> path = astar.search();
			//cerr << "npath: " << path.size() << endl; //// debug
			s.bots(0).fission(Vec3{1, 0, 0}, 0);
			s.commit();
			vector<command_t> cmds = create_move_commands(path);
			::execute(s, i, cmds);
#ifdef DEBUG_PRINT
			cerr << "pos of bot: " << i << ", " << s.bots(i).pos() << endl; ///// debug
			//cerr << "num of bots: " << s.num_bots() << endl; ///// debug
#endif
			astar.set_volatile_map(s.bots(i).pos());
		}
	}
	
	void gather() {
#ifdef DEBUG_PRINT
		cerr << "manager.gather" << endl; ///// debug
#endif
		//for (int i = 1; i < get_size(); i++) {
		while (s.num_bots() > 1) {
			for (int j = 2; j < s.num_bots(); j++)
				astar.set_volatile_map(s.bots(j).pos());
			//cerr << s.bots(1).pos() << endl; ///// debug
			astar.set(s.bots(1).pos(), Vec3{0, 1, 0});
			astar.set_volatile_map(Vec3{0, 0, 0});
			vector<Vec3> path = astar.search();
			vector<command_t> cmds = create_move_commands(path);
			::execute(s, 1, cmds);
#ifdef DEBUG_PRINT
			cerr << "bots(0).pos in gather: " << s.bots(0).pos() << endl; ///// debug
#endif
			s.bots(0).fusion_p(Vec3{0, 1, 0});
			s.bots(1).fusion_s(Vec3{0, -1, 0});
			s.commit();
			astar.clear_volatile_map();
		}
	}

	int push(const vector<command_t>& cmds) {
		//if (!agents[current_agent].cmds.empty()) execute();
		agents[current_agent].cmds = cmds;
		current_agent++;
		current_agent %= agents.size();
		if (!agents[current_agent].cmds.empty()) execute();
		return 0;
	}

	int execute() {
#ifdef DEBUG_PRINT
		cerr << "manager.execute" << endl; ///// debug
#endif
		bool changed = false;
		do {
			changed = false;
			for (auto& agent : agents) {
				command_t cmd;
				vector<int> args;
				int idx = 0;
				string c;
				if (!agent.cmds.empty()) {
					idx = agent.idx;
					cmd = agent.cmds.front();
					c = cmd.first;
					args = cmd.second;
					agent.cmds.erase(agent.cmds.begin());
					changed = true;
				} else continue;
				if (c == "smove") {
					s.bots(idx).smove(Vec3{args[0], args[1], args[2]});
				} else if (c == "fill") {
					s.bots(idx).fill(Vec3{args[0], args[1], args[2]});
				} else if (c == "flip") {
					s.bots(idx).flip();
				} else if (c == "wait") {
					s.bots(idx).wait();
				} else if (c == "halt") {
					s.bots(idx).halt();
				} else {
					;
				}
			}
			s.commit();
		} while (changed);
		for (auto& agent : agents) agent.cmds.clear();
		astar.clear_volatile_map();
		for (int i = 0; i < s.num_bots(); i++)
			astar.set_volatile_map(s.bots(i).pos());

		return 0;
	}

	int get_current() const { return current_agent; }

	int get_size() const { return agents.size(); }
};


int solver(VoxelGrid& v, const string& out_trace)
{
	int R = v.r();
	cerr << "Start solver: R=" << R << ", bots=" << NBOTS << ", voxels/bot=" << VOXELS_PER_BOT << endl; ///

	State s(R, 40);
	AStar astar(s);

	BotManager manager(NBOTS, v, s, astar);
	//manager.wave(NROWS, NCOLS);
	manager.wave();

	vector<command_t> all_cmds{};

	vector<command_t> cmds;
#if 0
	cmds.push_back(make_pair("flip", vector<int>{}));
	execute(s, 0, cmds);
#endif
	
	Vec3 pos(s.bots(0).pos());

	bool need_loop = false;
	bool changed = false;
	int rev = 1;
	do {
		for (;;) {
			need_loop = false;
			changed = false;
			//for (int y = 0; y < R - 1; y++) {
			for (int y = (rev == 1 ? 0 : R - 1); y != (rev == 1 ? R : -1); y += rev) {
				int num_ungrounded_cell = 0;
				int prev_num_ungrounded_cell = 0;
				//bool has_cell = false;
				vector<int> waited_bots;
				do {
					//has_cell = false;
					prev_num_ungrounded_cell = num_ungrounded_cell;
					num_ungrounded_cell = 0;
					vector<Vec3> target_voxels;
					for (int x = 1; x < R - 1; x++) {
						for (int z = 1; z < R - 1; z++) {
							if (v(z, y, x) == 1) {
								if (!is_grounded(s.matrix(), x, y, z)) {
									num_ungrounded_cell++;
									continue;
								}
								target_voxels.push_back(Vec3{x, y, z});
							}
						}
					}
					//random_shuffle(target_voxels.begin(), target_voxels.end());
					//for (const Vec3& target : target_voxels) {
					int nbots = manager.get_size();
					vector<Vec3> path;
					while (!target_voxels.empty()) {
						vector<int> removed_items;
						int offset_size = (target_voxels.size() - 1) / nbots + 1;
						int removed_cells = 0;
						for (int i = 0; i < offset_size; i++) {
							for (int n = 0; n < nbots; n++) {
								if (i + offset_size * n >= target_voxels.size()) continue;
								if (target_voxels.size() - removed_cells <= manager.get_current() * VOXELS_PER_BOT) {
									cmds.clear();
									//cmds.push_back(make_pair("wait", vector<int>{}));
									int man_idx = manager.get_current() / 10;
									int home_y = manager.get_current() % 10;
									int home_x = 0;
									int home_z = 0;
									switch (man_idx) {
									case 0:
										home_x = 0;
										home_z = 0;
										break;
									case 1:
										home_x = R - 1;
										home_z = 0;
										break;
									case 2:
										home_x = 0;
										home_z = R - 1;
										break;
									case 3:
										home_x = R - 1;
										home_z = R - 1;
										break;
									default:
										break;
									}
									astar.set(s.bots(manager.get_current()).pos(), Vec3{home_x, home_y, home_z});
									path = astar.search();
									for (const auto& pos : path) astar.set_volatile_map(pos);
									for (int k = 0; k < manager.get_size(); k++) astar.set_volatile_map(s.bots(k).pos());
									cmds = create_move_commands(path);
									manager.push(cmds);
									continue;
								}
								const Vec3& target = target_voxels[i + offset_size * n];
#ifdef DEBUG_PRINT
								cerr << "target: " << target << endl; ///// debug
#endif
								astar.set(s.bots(manager.get_current()).pos(), target + Vec3{0, rev, 0});
#ifdef DEBUG_PRINT
								cerr << "bot index: " << manager.get_current() << endl; ///// debug
#endif
								path = astar.search();
								if (target + Vec3(0, rev, 0) != path.back()) {
									cmds.clear();
									cmds.push_back(make_pair("wait", vector<int>{}));
									waited_bots.push_back(manager.get_current());
									if (waited_bots.size() >= manager.get_size()) {
										cerr << "Error: all bots waited" << endl;
										exit(1);
									}
								} else {
									const vector<Vec3>& volatile_map(astar.get_volatile_map());
									if (find(volatile_map.begin(), volatile_map.end(), path.back() - Vec3{0, rev, 0}) != volatile_map.end()) {
										cmds.clear();
										cmds.push_back(make_pair("wait", vector<int>{}));
										waited_bots.push_back(manager.get_current());
									} else {
										for (const auto& pos : path) astar.set_volatile_map(pos);
										astar.set_volatile_map(path.back() - Vec3{0, rev, 0});
										cmds = create_move_commands(path);
										cmds.push_back(make_pair("fill", vector<int>{0, -rev, 0}));
										v(target.z, target.y, target.x) = 0;
										changed = true;
										removed_items.push_back(i + offset_size * n);
										waited_bots.clear();
										removed_cells++;
//#ifdef DEBUG_PRINT
										cerr << "filled voxel: " << path.back() - Vec3{0, rev, 0} << endl; ///// debug
//#endif
									}
								}
								int ret = manager.push(cmds);
								all_cmds.insert(all_cmds.end(), cmds.begin(), cmds.end());
							}
						}
						sort(removed_items.begin(), removed_items.end());
						for (auto p = removed_items.rbegin(); p != removed_items.rend(); ++p) {
							target_voxels.erase(target_voxels.begin() + *p);
						}
					}
					manager.execute();
#ifdef DEBUG_PRINT
					cerr << "rev, nvoxcles, ungrounded: " << rev << ", " << target_voxels.size() << ", " << num_ungrounded_cell << ", " << prev_num_ungrounded_cell << endl; ///// debug
#endif
					//astar.clear_volatile_map();
					//for (int i = 0; i < s.num_bots(); i++) astar.set_volatile_map(s.bots(i).pos());
					if (num_ungrounded_cell > 0 && num_ungrounded_cell == prev_num_ungrounded_cell)
						need_loop = true;
				} while (num_ungrounded_cell > 0 && num_ungrounded_cell != prev_num_ungrounded_cell);
			}
			if (!need_loop) break;
			rev *= -1;
		}
	} while (need_loop);
	//} while (need_loop && changed);
	
	astar.set(s.bots(0).pos(), Vec3{0, 0, 0});
	vector<Vec3> path = astar.search();
	cmds = create_move_commands(path);
#if 0
	cmds.push_back(make_pair("flip", vector<int>{}));
#endif
	execute(s, 0, cmds);
#ifdef DEBUG_PRINT
	cerr << "final position: " << s.bots(0).pos() << ", " << s.bots(0).bid() << endl; //// debug
#endif
	manager.gather();
	cmds.clear();
	cmds.push_back(make_pair("halt", vector<int>{}));
	execute(s, 0, cmds);
	all_cmds.insert(all_cmds.end(), cmds.begin(), cmds.end());
	//for (const auto& cmd : all_cmds) cerr << cmd.first << endl; ///// debug
#ifdef DEBUG_PRINT
	cerr << "commands: " << all_cmds.size() << endl; ///// debug
#endif

	s.export_trace(out_trace);

	cerr << "Finished" << endl; ///
	
	return 0;
}

int main(int argc, char* argv[])
{
	if (argc < 3) {
		cerr << "Usage " << argv[0] << " input-model-file output-trace-file" << endl;
		exit(1);
	}
	
	VoxelGrid v = read_data(argv[1]);

	solver(v, argv[2]);

	return 0;
}
