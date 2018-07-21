#include <iostream>
#include <vector>
#include <tuple>
#include <queue>
#include <cassert>
#include <fstream>
#include <stack>
#include <map>

using namespace std;

#include "nbtasm.hpp"
#include "backward.hpp"

using namespace detail;

const bool DEBUG = false;

const uint8_t RIGHT = 1;
const uint8_t LEFT = 2;
const uint8_t UP = 4;
const uint8_t DOWN = 8;
const uint8_t AHEAD = 16;
const uint8_t BACK = 32;

const array<int, 6> dirs = {
  RIGHT, LEFT, UP, DOWN, AHEAD, BACK
};

const array<Vec3, 6> dp = {
  Vec3{1, 0, 0},
  Vec3{-1, 0, 0},
  Vec3{0, 1, 0},
  Vec3{0, -1, 0},
  Vec3{0, 0, 1},
  Vec3{0, 0, -1},
};

const Vec3 ORIGIN(0, 0, 0);

array<Vec3, 18> near_coordinate_diff;

void construct_near_coordinate_diff() {
  int index = 0;
  for (int y = 1; y >= -1; y--) {
    for (int x = -1; x <= 1; x++) {
      for (int z = -1; z <= 1; z++) {
        const int dist = abs(x) + abs(y) + abs(z);
        if (0 < dist && dist <= 2) {
          near_coordinate_diff[index++] = Vec3 {x, y, z};
        }
      }
    }
  }
}

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
			//v(cnt / (R * R), (cnt / R) % R, cnt % R) = (x & 0x80) ? 1 : 0;
			//v(cnt % R, (cnt / R) % R, cnt / (R * R)) = (x & 0x80) ? 1 : 0;
			v(cnt % R, (cnt / R) % R, cnt / (R * R)) = static_cast<uint8_t>(x & 0x1);
			x >>= 1;
			cnt++;
		}
	}

	return v;
}

int write_data(string fname, const VoxelGrid& v)
{
	fstream fs(fname.c_str(), ios_base::out | ios_base::binary);

	int R = v.r();
	unsigned char x = static_cast<char>(R);
	fs.write((char*)&x, 1);
	int cnt = 0;
	for (int i = 0; i < (R * R * R - 1) / 8 + 1; i++) {
		unsigned char x = 0;
		for (int j = 0; j < 8; j++) {
			x >>= 1;
			if (cnt < R * R * R)
				x |= v(cnt % R, (cnt / R) % R, cnt / (R * R)) ? 0x80 : 0x0;
			cnt++;
		}
		fs.write((char*)&x, 1);
	}

	return 0;
}

// library of Vec

bool in_range(const Vec3 v, const int r) {
  return (0 <= v.x && v.x < r) &&
    (0 <= v.y && v.y < r) &&
    (0 <= v.z && v.z < r);
}

void bfs(const VoxelGrid &grid,
         const Vec3 start,
         VoxelGrid &root) {

  const int r = grid.r();

  vector<bool> visited(r * r * r, false);

  queue<Vec3> que;
  que.push(start);
  visited[start.encode(r)] = true;

  while (!que.empty()) {
    auto val = que.front();
    que.pop();

    for (int i = 0; i < 6; i++) {
      const Vec3 np = val + dp[i];
      const int enc = np.encode(r);

      if (in_range(np, r) &&
          !visited[enc] &&
          grid(np)) {

        visited[enc] = true;
        root(val) |= dirs[i];
        que.push(np);
      }
    }
  }
}

bool is_leaf(const VoxelGrid &root, Vec3 pos) {
  return root(pos) == 0;
}

// root は tree と仮定して良い
void dfs(VoxelGrid &root, const Vec3 pos, vector<Vec3> &order) {
  stack<Vec3> s;
  s.push(pos);

  while (!s.empty()) {
    Vec3 pos = s.top();
    s.pop();

    order.push_back(pos);

    if (!is_leaf(root, pos)) {
      for (int i = 0; i < 6; i++) {
        if (root(pos) & dirs[i]) {
          s.push(pos + dp[i]);
          if (DEBUG) cout << pos << " -> " << (pos + dp[i]) << endl;
        }
      }
    }
  }
  reverse(order.begin(), order.end());
}

void generate_order_list(vector<vector<Vec3>> &order_list,
                         const VoxelGrid &grid,
                         VoxelGrid &root)
{
  const int r = grid.r();

  // y == 0 平面でのvisited
  vector<bool> visited(r * r, false);

  for (int z = 0; z < r; z++) {
    for (int x = 0; x < r; x++) {
      const Vec3 pos{x, 0, z};

      if (!visited[z * r + x] && (1 == grid(pos))) {
        visited[z * r + x] = true;

        bfs(grid, pos, root);
        vector<Vec3> order;
        dfs(root, pos, order);

        for (auto &p : order) {
          if (p.y == 0) {
            visited[p.z * r + p.x] = true;
          }
        }
        order_list.push_back(order);
      }
    }
  }
}

// dist, dir
pair<int, Vec3> select_dir(const Vec3 from, const Vec3 to) {
  const int dx = abs(from.x - to.x);
  const int dy = abs(from.y - to.y);
  const int dz = abs(from.z - to.z);
  const int max_d = max(dx, max(dy, dz));

  if (dx == max_d) {
    if (from.x < to.x) {
      return make_pair(to.x - from.x, Vec3{1, 0, 0});
    } else {
      return make_pair(from.x - to.x, Vec3{-1, 0, 0});
    }
  } else if(dy == max_d) {
    if (from.y < to.y) {
      return make_pair(to.y - from.y, Vec3 {0, 1, 0});
    } else {
      return make_pair(from.y - to.y, Vec3 {0, -1, 0});
    }
  } else {
    if (from.z < to.z) {
      return make_pair(to.z - from.z, Vec3 {0, 0, 1});
    } else {
      return make_pair(from.z - to.z, Vec3 {0, 0, -1});
    }
  }
}

// TODO: 詰むケースが存在する
void move(const Vec3 from, const Vec3 to, State &state) {
  if (DEBUG) cout << "from -> to: " << from << " " << to << endl;
  Vec3 cur = from;
  while (cur != to) {
    Vec3 dir;
    int max_dist;
    tie(max_dist, dir) = select_dir(cur, to);
    max_dist = min(max_dist, 15);

    int dist = 0;
    Vec3 diff = {0, 0, 0};

    if (DEBUG) cout << "dir: " << dir << endl;
    if (DEBUG) cout << "max_dist: " << max_dist << endl;

    while (dist < max_dist) {
      Vec3 next = cur + dir;
      if (state.matrix()(next) == 1) {
        break;
      }
      diff += dir;
      dist++;
    }

    if (DEBUG) cout << "diff: " << diff << endl;
    assert(dist > 0);

    state.bots(0).smove(diff);
    state.commit();

    cur += diff;
    if (DEBUG) cout << "---" << cur << endl;
  }
}

void move2(const Vec3 from, const Vec3 to, State &state) {
  const int r = state.matrix().r();
  vector<bool> visited(r * r * r, false);
  visited[from.encode(r)] = true;
  queue<Vec3> que;
  que.push(from);
  map<Vec3, Vec3> previous;
  previous[from] = Vec3 {-1, -1, -1};

  while (!que.empty()) {
    Vec3 val = que.front();
    que.pop();

    if (val == to) {
      vector<Vec3> restored;
      while (previous[val].x != -1) {
        const Vec3 diff = val - previous[val];
        restored.push_back(diff);
        val = previous[val];
      }
      reverse(restored.begin(), restored.end());
      for (Vec3 v : restored) {
       if (DEBUG) cout << "diff: " << v << endl;
        state.bots(0).smove(v);
        state.commit();
      }
      return;
    }

    for (Vec3 d : dp) {
      Vec3 next = val;
      for (int dist = 1; dist < 15; dist++) {
        next += d;
        if (!in_range(next, r) ||
            state.matrix()(next) == 1) {
          break;
        }
        if (!visited[next.encode(r)]) {
          visited[next.encode(r)] = true;
          previous[next] = val;
          que.push(next);
        }
      }
    }
  }
  assert(false);
}

void collect_neighbor(const Vec3 pos, const State &state,
                      vector<Vec3> &diff_list) {
  const int r = state.matrix().r();
  for (const Vec3 diff : near_coordinate_diff) {
    const Vec3 np = pos + diff;
    if (in_range(np, r) &&
        state.matrix()(np) == 1) {
      diff_list.push_back(diff);
    }
  }
}

void delete_around(const Vec3 pos, State &state) {
  vector<Vec3> diff_list;
  collect_neighbor(pos, state, diff_list);

  for (Vec3 diff : diff_list) {
    state.bots(0).empty(diff);
    if (DEBUG) cout << "EMPTY: " << (pos + diff) << endl;
    state.commit();
  }
}

void delete_target(const Vec3 target, State &state) {
  const Vec3 diff = target - state.bots(0).pos();
  state.bots(0).empty(diff);
  if (DEBUG) cout << "EMPTY: " << target << endl;
  state.commit();
}

Vec3 get_empty_voxel(const Vec3 target, const State &state) {
  const int r = state.matrix().r();
  for (const Vec3 diff : near_coordinate_diff) {
    Vec3 next = target + diff;
    if (in_range(next, r) &&
        state.matrix()(next) == 0) {
      return next;
    }
  }
  assert(false);
}

// 現在の座標からorderの順に埋めて、終了posを返す
void solve(const VoxelGrid &grid, State &state,
      const vector<Vec3> &order) {
  const int r = grid.r();

  int turn = 0;
  for (const Vec3 pos : order) {
    if (turn++ % 1000 == 0) {
      cout << turn << " / " << order.size() << endl;
    }

    if (state.matrix()(pos) == 1) {
      Vec3 target = get_empty_voxel(pos, state);
      if (DEBUG) {
        cout << "cur: " << state.bots(0).pos() << endl;
        cout << "pos: " << pos << endl;
        cout << "target: " << target << endl;
      }
      move2(state.bots(0).pos(), target, state);
      delete_target(pos, state);
      // delete_around(state.bots(0).pos(), state);
      if (DEBUG) cout << "---" << endl;
    }
  }
}

void init() {
  construct_near_coordinate_diff();
}

int main(int argc, char *argv[])
{
  init();

  string input_file_name(argv[1]);
  string output_file_name(argv[2]);

  VoxelGrid grid = read_data(input_file_name);

  VoxelGrid root(grid.r());
  for (int x = 0; x < grid.r(); x++) {
    for (int y = 0; y < grid.r(); y++) {
      for (int z = 0; z < grid.r(); z++) {
        root(z, y, x) = 0;
      }
    }
  }

  vector<vector<Vec3>> order_list;
  generate_order_list(order_list, grid, root);

  if (DEBUG) {
    for (auto &v : order_list) {
      cout << "[";
      for (auto &item : v) {
        cout << item << ", ";
      }
      cout << "]" << endl;
    }
  }

  State state(grid, 1);
  int count = 0;

  for (const auto &lst : order_list) {
    // cout << "order-list: " << count++ << " / " << order_list.size() << endl;
    solve(grid, state, lst);
  }
  const int r = state.matrix().r();

  if (state.bots(0).pos() != ORIGIN) {
    move2(state.bots(0).pos(), ORIGIN, state);
    // cout << state.matrix() << endl;
  }
  state.bots(0).halt();
  state.commit();

  export_backward_trace(output_file_name, state);
}
