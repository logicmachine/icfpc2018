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
#include "model_io.hpp"

using namespace detail;

const bool DEBUG = false;

const int MAX_NANOBOT = 40;

const uint8_t RIGHT = 1;
const uint8_t LEFT = 2;
const uint8_t UP = 4;
const uint8_t DOWN = 8;
const uint8_t AHEAD = 16;
const uint8_t BACK = 32;

uint8_t rev_dir(const uint8_t dir) {
  switch (dir) {
  case RIGHT: return LEFT;
  case LEFT: return RIGHT;
  case UP: return DOWN;
  case DOWN: return UP;
  case AHEAD: return BACK;
  case BACK: return AHEAD;
  }
}

const array<int, 6> dirs = {
  RIGHT, LEFT, UP, DOWN, AHEAD, BACK
};

const array<string, 6> dirs_str = {
  "RIGHT", "LEFT", "UP", "DOWN", "AHEAD", "BACK"
};

const array<Vec3, 6> dp = {
  Vec3{1, 0, 0},
  Vec3{-1, 0, 0},
  Vec3{0, 1, 0},
  Vec3{0, -1, 0},
  Vec3{0, 0, 1},
  Vec3{0, 0, -1},
};

int get_active_index(const Vec3 &v) {
  assert(v.mlen() == 1);

  if (v.x != 0) return 0;
  else if (v.y != 0) return 1;
  else return 2;
}

// neighbor_2d[i][xyz]
array<array<Vec3, 4>, 3> neighbor_2d;

void construct_neighbor_2d () {
  neighbor_2d[0][0] = Vec3 {0, -1, -1};
  neighbor_2d[0][1] = Vec3 {0, -1, 1};
  neighbor_2d[0][2] = Vec3 {0, 1, -1};
  neighbor_2d[0][3] = Vec3 {0, 1, 1};
  neighbor_2d[1][0] = Vec3 {-1, 0, -1};
  neighbor_2d[1][1] = Vec3 {-1, 0, 1};
  neighbor_2d[1][2] = Vec3 {1, 0, -1};
  neighbor_2d[1][3] = Vec3 {1, 0, 1};
  neighbor_2d[2][0] = Vec3 {-1, -1, 0};
  neighbor_2d[2][1] = Vec3 {-1, 1, 0};
  neighbor_2d[2][2] = Vec3 {1, -1, 0};
  neighbor_2d[2][3] = Vec3 {1, 1, 0};
}

array<Vec3, 27> neighbor_3d;

void construct_neighbor_3d() {
  int index = 0;
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      for (int dz = -1; dz <= 1; dz++) {
        neighbor_3d[index++] = Vec3 {dx, dy, dz};
      }
    }
  }
}

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

array<Vec3, MAX_NANOBOT> goal_nanobot_list;

void construct_nanobot_goal() {
  int index = 0;
  Vec3 goal {0, 0, 0};
  for (int x = 0; x < 5; x++) {
    for (int y = 0; y < 8; y++) {
      goal_nanobot_list[index] = goal;
      if (index % 5 == 4) {
        goal.y++;
      } else if (index % 10 < 4) {
        goal.x++;
      } else {
        goal.x--;
      }
      index++;
    }
  }
}

bool in_range(const Vec3 v, const int r) {
  return (0 <= v.x && v.x < r) &&
    (0 <= v.y && v.y < r) &&
    (0 <= v.z && v.z < r);
}

struct Node {
  uint8_t dir;
  uint8_t orig;
  uint8_t marked;
  uint64_t depth;
  uint64_t visited;
};

void print_dir(uint8_t dir) {
  cout << static_cast<int>(dir) << ": ";
  cout << "{";
  for (int i = 0; i < 6; i++) {
    if (dirs[i] & dir) {
      cout << dirs_str[i] << ", ";
    }
  }
  cout << "}" << endl;
}

void dump(const Node &n) {
  cout << "-- dump node: --" << endl;
  print_dir(n.dir);
  cout << "orig: " << static_cast<int>(n.orig) << endl;
  cout << "marked: " << static_cast<int>(n.marked) << endl;
  cout << "depth: " << n.depth << endl;
  cout << "visited: " << n.visited << endl;
  cout << "-- end of dump node --" << endl;
}

class GridGraph {

private:
	int m_r;
  int m_gc; // m_gc と同じならtrue, そうでないならfalse
	std::vector<Node> m_grid;

  void set_single_trajectory(Vec3 pos, Vec3 diff) {
    while (diff.x > 0) {
      diff.x--; visit(pos + diff);
    }
    while (diff.x < 0) {
      diff.x++; visit(pos + diff);
    }
    while (diff.y > 0) {
      diff.y--; visit(pos + diff);
    }
    while (diff.y < 0) {
      diff.y++; visit(pos + diff);
    }
    while (diff.z > 0) {
      diff.z--; visit(pos + diff);
    }
    while (diff.z < 0) {
      diff.z++; visit(pos + diff);
    }
  }

  void reset_single_trajectory(Vec3 pos, Vec3 diff) {
    while (diff.x > 0) {
      diff.x--; forget(pos + diff); 
    }
    while (diff.x < 0) {
      diff.x++; forget(pos + diff);
    }
    while (diff.y > 0) {
      diff.y--; forget(pos + diff);
    }
    while (diff.y < 0) {
      diff.y++; forget(pos + diff);
    }
    while (diff.z > 0) {
      diff.z--; forget(pos + diff);
    }
    while (diff.z < 0) {
      diff.z++; forget(pos + diff);
    }
  }

public:
  GridGraph()
		: m_grid()
    , m_gc(1)
	{ }

	explicit GridGraph(int r)
		: m_grid(r * r * r)
		, m_r(r)
    , m_gc(1)
	{ }

	Node &operator()(int i, int j, int k) noexcept {
		const int r = m_r;
		return m_grid[i * r * r + j * r + k];
	}
	Node &operator()(const Vec3 &v){
		const int r = m_r;
		return m_grid[v.z * r * r + v.y * r + v.x];
	}

  void visit(int i, int j, int k) {
    // assert(!visited(i, j, k));
		const int r = m_r;
		m_grid[i * r * r + j * r + k].visited = m_gc;
    if (DEBUG) cout << "mark: " << Vec3 {k, j, i} << endl;
  }

  void visit(const Vec3 &v) {
    // assert(!visited(v));
		const int r = m_r;
		m_grid[v.z * r * r + v.y * r + v.x].visited = m_gc;
    if (DEBUG) cout << "mark: " << v << endl;
  }

  void forget(int i, int j, int k) {
    // assert(visited(i, j, k));
		const int r = m_r;
		m_grid[i * r * r + j * r + k].visited = 0;
  }

  void forget(const Vec3 &v) {
    // assert(visited(v));
		const int r = m_r;
		m_grid[v.z * r * r + v.y * r + v.x].visited = 0;
  }

  bool visited(int i, int j, int k) const {
		const int r = m_r;
		return m_grid[i * r * r + j * r + k].visited == m_gc;
  }

  bool visited(const Vec3 &v) const {
		const int r = m_r;
		return m_grid[v.z * r * r + v.y * r + v.x].visited == m_gc;
  }

  void initialize_visited() {
    m_gc++;
  }

  void set_trajectory(const Vec3 &pos, const Command &cmd) {

    assert(cmd.type == CommandType::SMove ||
           cmd.type == CommandType::LMove);

    switch (cmd.type) {
    case CommandType::SMove:
      set_single_trajectory(pos, cmd.smove_lld());
      visit(pos + cmd.smove_lld());
      break;
    case CommandType::LMove:
      set_single_trajectory(pos, cmd.lmove_sld1());
      set_single_trajectory(pos + cmd.lmove_sld1(), cmd.lmove_sld2());
      visit(pos + cmd.lmove_sld1() + cmd.lmove_sld2());
      break;
    }
  }

  void reset_trajectory(const Vec3 &pos, const Command &cmd) {
    assert(cmd.type == CommandType::SMove ||
           cmd.type == CommandType::LMove);

    forget(pos);

    switch (cmd.type) {
    case CommandType::SMove:
      reset_single_trajectory(pos, cmd.smove_lld());
      break;
    case CommandType::LMove:
      reset_single_trajectory(pos, cmd.lmove_sld1());
      reset_single_trajectory(pos + cmd.lmove_sld1(), cmd.lmove_sld2());
      break;
    }
  }

	int r() const noexcept { return m_r; }
	int size() const noexcept { return m_r; }

  bool empty () const noexcept {
    for (int i = 0; i < r(); i++) {
      for (int j = 0; j < r(); j++) {
        for (int k = 0; k < r(); k++) {
          if ((i, j, k) == m_gc) {
            return false;
          }
        }
      }
    }
    return true;
  }
};

void generate_nanobot(const int max_nanobot, State &state) {
  dump(state);
  for (int i = 0; i < max_nanobot - 1; i++) {
    state.bots(i).fission(goal_nanobot_list[i + 1] - goal_nanobot_list[i], max_nanobot - 2 - i);
    state.commit();
    dump(state);
  }
}

void collect_nanobot(const int max_nanobot, State &state) {
  for (int i = max_nanobot - 1; i > 0; i--) {
    if (DEBUG) dump(state);
    state.bots(i).fusion_s(goal_nanobot_list[i] - goal_nanobot_list[i - 1]);
    state.bots(i - 1).fusion_p(goal_nanobot_list[i - 1] - goal_nanobot_list[i]);
    state.commit();
  }
  if (DEBUG) dump(state);
}

void bfs(const vector<Vec3> &start_list,
         GridGraph &graph,
         vector<Vec3> &order_list) {

  const int r = graph.r();

  graph.initialize_visited();

  queue<pair<uint64_t, Vec3>> que;
  for (const Vec3 &start : start_list) {
    que.push(make_pair(1, start));
    graph.visit(start);
  }

  while (!que.empty()) {
    Vec3 val;
    uint64_t depth;
    tie(depth, val) = que.front();
    que.pop();

    // cout << "--- start ---" << endl;
    // dump(graph(val));

    graph(val).depth = depth;

    order_list.push_back(val);

    for (int i = 0; i < 6; i++) {
      const Vec3 np = val + dp[i];

      if (in_range(np, r) &&
          !graph.visited(np) &&
          graph(np).orig) {

        graph.visit(np);
        graph(val).dir |= dirs[i];
        que.push(make_pair(depth + 1, np));
      }
    }
    // dump(graph(val));
    // cout << "--- end ---" << endl;
  }
}

void struct_order_list(GridGraph &graph, vector<Vec3> &order_list) {
  const int r = graph.r();
  vector<Vec3> start_list;
  for (int x = 0; x < r; x++) {
    for (int z = 0; z < r; z++) {
      if (graph(z, 0, x).orig) {
        start_list.push_back(Vec3 {x, 0, z});
      }
    }
  }
  bfs(start_list, graph, order_list);
}

enum class ActionType {
  None,
  Move,
  FillBelow,
  Complete,
};

string to_string(const ActionType type) {
  switch(type) {
  case ActionType::None: return "None";
  case ActionType::Move: return "Move";
  case ActionType::FillBelow: return "FillBelow";
  case ActionType::Complete: return "Complete";
  }
}

struct ActionState {
  ActionType type;
  Vec3 v1;
  int counter;
  int target_depth;
};

ostream &operator<<(ostream &out, const ActionState &action) {
  out << "(" << to_string(action.type) << " " << action.v1 << " " <<
    action.counter << " " << action.target_depth << ")";
  return out;
}

bool complete(const vector<ActionState> &state_list) {
  for (auto &s : state_list) {
    if (s.type != ActionType::Complete) {
      return false;
    }
  }
  return true;
}

Vec3 select_target_diff(GridGraph &graph, State &state, const Vec3 &target_fill) {

  const int r = graph.r();
  // 6近傍で、empty かつ bfs treeが深さ1違いの部分があれば、採用
  for (int i = 0; i < 6; i++) {
    const Vec3 target = target_fill + dp[i];
    /**
    if (DEBUG) cout << "check1 " << target << endl;
    if (DEBUG) {
      cout << "in_range: " << in_range(target, r) << endl;
      cout << "target: " << endl;
      dump(graph(target));
      cout << "target fill:" << endl;
      dump(graph(target_fill));
      cout << endl << endl;
    }
    **/
    if (in_range(target, r) && graph(target).orig &&
        !graph(target).marked && (graph(target_fill).dir & dirs[i]) &&
        graph(target).depth - 1 == graph(target_fill).depth) {
      // if (DEBUG) cout << "sucess.." << endl;
      return dp[i];
    }
    // if (DEBUG) cout << "fail.." << endl;
  }

  // 18近傍で、emptyの場所があれば、そこを採用
  for (const Vec3 diff : near_coordinate_diff) {
    const Vec3 target = target_fill + diff;
    /**
    if (DEBUG) cout << "check2 " << target << endl;
    if (DEBUG) {
      cout << "in_range: " << in_range(target, r) << endl;
      cout << "target: " << endl;
      dump(graph(target));
      cout << "target fill:" << endl;
      dump(graph(target_fill));
      cout << endl << endl;
    }
    **/
    if (in_range(target, r) && !graph(target).orig) {
      // if (DEBUG) cout << "sucess.." << endl;
      return diff;
    }
    // if (DEBUG) cout << "fail.." << endl;
  }
  assert(false);
}

Vec3 decide_next_pos(ActionState &action, State &state, const int bid,
                     vector<Vec3> &order_list, GridGraph &graph) {
  while (!order_list.empty() && graph(order_list.back()).marked) {
    order_list.pop_back();
  }

  // 消すやつが1つ以上残っている
  if (!order_list.empty()) {
    const Vec3 target_fill = order_list.back();
    order_list.pop_back();
    assert(!graph(target_fill).marked);

    // if (DEBUG) cout << "target_fill: " << target_fill << endl;

    const Vec3 diff = select_target_diff(graph, state, target_fill);
    const Vec3 target = target_fill + diff;
    // if (DEBUG) cout << "target: " << target << endl;
    assert(in_range(target, graph.r()));

    Node &nt = graph(target);
    action.target_depth = graph(target_fill).depth;
    // if (DEBUG) cout << "target node: " << endl;
    // if (DEBUG) dump(graph(target));
    // if (DEBUG) dump(graph(target_fill));

    for (Vec3 d : near_coordinate_diff) {
      const Vec3 target_another = target + d;

      if (in_range(target_another, graph.r())) {
        Node &na = graph(target_another);
        // if (DEBUG) cout << "target another: " << endl;
        // dump(na);

        if (na.orig && !na.marked && action.target_depth == na.depth) {
          na.marked = true;
        }
      }
    }
    return target;
  } else {
    // Fill 対象の Box がないので、お家に帰る
    return goal_nanobot_list[bid];
  }
}

Command act_move(ActionState &action, State &state, const int bid,
                 vector<Vec3> &order_list, GridGraph &graph);

Command act_fill_below(ActionState &action, State &state, const int bid,
                       vector<Vec3> &order_list, GridGraph &graph) {
  assert(action.type == ActionType::FillBelow);

  for (Vec3 diff : near_coordinate_diff) {
    const Vec3 pos = action.v1 + diff;
    // この位置にあるtarget_depthのvoxelは全て自分によってmarkedされているので、
    if (in_range(pos, graph.r()) && graph(pos).orig &&
        !graph.visited(pos) && !state.matrix()(pos) &&
        graph(pos).depth == action.target_depth) {

      graph.visit(pos);
      return Command(CommandType::Fill).fill_nd(diff);
    }
  }

  // fill が終わっているので、次の場所にmoveする
  Vec3 next_pos = decide_next_pos(action, state, bid, order_list, graph);

  action.type = ActionType::Move;
  action.v1 = next_pos;

  return act_move(action, state, bid, order_list, graph);
}

Command act_move(ActionState &action, State &state, const int bid,
                 vector<Vec3> &order_list, GridGraph &graph) {

  if (DEBUG) cout << "move: " << state.bots(bid).pos() << " -> " << action.v1 << endl;

  // visited[idx] == counter の場合が、true
  static array<uint64_t, 256 * 256 * 256> visited;
  static uint64_t counter = 0;
  map<Vec3, Vec3> previous;
  previous[state.bots(bid).pos()] = Vec3 {-1, -1, -1};

  if (action.v1 == state.bots(bid).pos()) {
    action.type = ActionType::FillBelow;
    act_fill_below(action, state, bid, order_list, graph);
  } else {

    counter++;

    if (DEBUG) cout << "counter =" << counter << endl;

    const int r = state.matrix().r();
    queue<Vec3> que;
    que.push(state.bots(bid).pos());

    while (!que.empty()) {
      Vec3 val = que.front();
      que.pop();

      // if (DEBUG) cout << "pop: " << val << endl;

      if (val == action.v1) {
        Vec3 just_prev;

        while (previous[val].x != -1) {
          if (DEBUG) cout << val << " -> " << endl;

          const Vec3 diff = val - previous[val];
          just_prev = val;
          val = previous[val];
        }
        if (DEBUG) cout << val << endl;
        // TODO: 毎回bfsは辛すぎるので、軌跡予約システムを入れる
        Command cmd = Command(CommandType::SMove).smove_lld(just_prev - state.bots(bid).pos());
        graph.set_trajectory(state.bots(bid).pos(), cmd);
        return cmd;

      } else {
        // if (DEBUG) cout << "enumerate neighbor" << endl;
        // s-move
        for (Vec3 d : dp) {
          Vec3 next = val;

          for (int dist = 1; dist <= 15; dist++) {
            next += d;

            /**
            cout << "  check " << next << endl;
            cout << "    in range: " << in_range(next, r) << endl;
            if (in_range(next, r)) {
              cout << "    already built: " << static_cast<int>(state.matrix()(next)) << endl;
              cout << "    already another bot's path: " << graph.visited(next) << endl;
            }
            cout << "  end of checking" << endl << endl;
            **/

            if (!in_range(next, r) ||
                (state.matrix()(next) == 1) ||
                graph.visited(next)) {
              break;
            }
            if (visited[next.encode(r)] != counter) {
              visited[next.encode(r)] = counter;
              previous[next] = val;
              que.push(next);
            }
          }
        }
      }
    }
  }
  return Command(CommandType::Wait);
}

Command act_none(ActionState &action, State &state, const int bid,
                 vector<Vec3> &order_list, GridGraph &graph) {
  assert(action.type == ActionType::None);

  Vec3 next_pos = decide_next_pos(action, state, bid, order_list, graph);

  if (DEBUG) cout << "next target: " << next_pos << endl;

  action.type = ActionType::Move;
  action.v1 = next_pos;

  return act_move(action, state, bid, order_list, graph);
}

void solve(GridGraph &graph, State &state, vector<Vec3> &order_list) {

  reverse(order_list.begin(), order_list.end());

  vector<ActionState> action_list(state.num_bots(), ActionState {ActionType::None, Vec3 {0, 0, 0}});

  int turn = 0;
  while (!complete(action_list)) {
    cout << endl << "turn: " << turn++ << endl;

    graph.initialize_visited();
    for (int bid = 0; bid < state.num_bots(); bid++) {
      if (DEBUG) cout << state.bots(bid) << endl;
      graph.visit(state.bots(bid).pos());
    }

    for (auto &action : action_list) cout << action << endl;
    if (DEBUG) dump(state);
    if (DEBUG) cout << "num_bot: " << state.num_bots() << endl;
    if (DEBUG) cout << "---" << endl;

    for (int bid = 0; bid < state.num_bots(); bid++) {
      switch (action_list[bid].type) {
      case ActionType::None:
        {
          Command cmd = act_none(action_list[bid], state, bid, order_list, graph);
          if (DEBUG) cout << "None: " << endl;
          if (DEBUG) cout << cmd << endl;
          state.bots(bid).exec(cmd);
          break;
        }
      case ActionType::Move:
        {
          Command cmd = act_move(action_list[bid], state, bid, order_list, graph);
          if (DEBUG) cout << "None: " << endl;
          if (DEBUG) cout << cmd << endl;
          state.bots(bid).exec(cmd);
          break;
        }

      case ActionType::FillBelow:
        {
          Command cmd = act_fill_below(action_list[bid], state, bid, order_list, graph);
          if (DEBUG) cout << "None: " << endl;
          if (DEBUG) cout << cmd << endl;
          state.bots(bid).exec(cmd);
          break;
        }

      case ActionType::Complete:
        {
          // waitしたいので、何もしない
          break;
        }
      }
    }
    state.commit();
  }
}

void init() {
  construct_near_coordinate_diff();
  construct_neighbor_2d();
  construct_neighbor_3d();
  construct_nanobot_goal();
}

int main(int argc, char *argv[])
{
  init();

  string input_file_name(argv[1]);
  string output_file_name(argv[2]);

  VoxelGrid grid = read_data(input_file_name);

  GridGraph graph(grid.r());

  for (int i = 0; i < grid.r(); i++) {
    for (int j = 0; j < grid.r(); j++) {
      for (int k = 0; k < grid.r(); k++) {
        graph(i, j, k).orig = grid(i, j, k);
      }
    }
  }

  vector<Vec3> order_list;

  struct_order_list(graph, order_list);

  // 順方向にすすめるなら、最初は空
  // 逆方向にすすめるなら、最初はoriginal graph
  State state(graph.r(), MAX_NANOBOT);

  try {

    generate_nanobot(MAX_NANOBOT, state);

    solve(graph, state, order_list);

    collect_nanobot(MAX_NANOBOT, state);

    state.bots(0).halt();
    state.commit();

  } catch (int error) {
  } 
  state.export_trace(output_file_name);
}
