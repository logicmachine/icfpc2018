#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <utility>
#include <queue>
#include <tuple>
#include <cstdint>

#include "../nbtasm/nbtasm.hpp"

using namespace std;

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
	}

	return path;
}

vector<pair<string, vector<int>>> create_move_commands(const vector<Vec3>& path)
{
	vector<Vec3> diffs;
	vector<Vec3> vs;
	vector<pair<string, vector<int>>> cmds;

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

int execute(State& s, const vector<pair<string, vector<int>>>& cmds)
{
	for (const auto& cmd : cmds) {
		string c = cmd.first;
		vector<int> args = cmd.second;
		if (c == "smove") {
			s.bots(0).smove(Vec3{args[0], args[1], args[2]});
		} else if (c == "fill") {
			s.bots(0).fill(Vec3{args[0], args[1], args[2]});
		} else if (c == "flip") {
			s.bots(0).flip();
		} else if (c == "halt") {
			s.bots(0).halt();
		} else {
			;
		}
		s.commit();
	}
	
	return 0;
}

int solver(VoxelGrid& v, const string& out_trace)
{
	int R = v.r();
	cout << "R: " << R << endl;

	State s(R, 20);
	//State s(v, 20);
	AStar astar(s);
	vector<pair<string, vector<int>>> all_cmds{};

	vector<pair<string, vector<int>>> cmds;
	cmds.push_back(make_pair("flip", vector<int>{}));
	execute(s, cmds);
	
	Vec3 pos(s.bots(0).pos());
	for (int y = 0; y < R - 1; y++) {
		for (int x = 1; x < R - 1; x++) {
			for (int z = 1; z < R - 1; z++) {
 				//if (s.matrix()(z, y, x) == 1) {
				if (v(z, y, x) == 1) {
					cerr << "Loc: " << x << ", " << y << ", " << z << endl; ///// debug
					astar.set(pos, Vec3{x, y + 1, z});
					pos = Vec3{x, y + 1, z};
					vector<Vec3> path = astar.search();
					cmds = create_move_commands(path);
					cmds.push_back(make_pair("fill", vector<int>{0, -1, 0}));
					execute(s, cmds);
					all_cmds.insert(all_cmds.end(), cmds.begin(), cmds.end());
					v(z, y, x) = 0;
				}
			}
		}
	}
	astar.set(pos, Vec3{0, 0, 0});
	vector<Vec3> path = astar.search();
	cmds = create_move_commands(path);
	cmds.push_back(make_pair("flip", vector<int>{}));
	cmds.push_back(make_pair("halt", vector<int>{}));
	execute(s, cmds);
	all_cmds.insert(all_cmds.end(), cmds.begin(), cmds.end());
	//for (const auto& cmd : all_cmds) cerr << cmd.first << endl; ///// debug
	cerr << "commands: " << all_cmds.size() << endl; ///// debug

#if 0
	for (int y = 0; y < R; y++) {
		for (int x = 1; x < R; x++) {
			for (int z = 1; z < R; z++) {
				if (v(z, y, x) == 1) cerr << "BLOCK: " << x << ", " << y << ", " << z << endl;
			}
		}
	}
#endif

	s.export_trace(out_trace + ".nbt");

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
