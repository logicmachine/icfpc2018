#include <array>

#include "../../nbtasm/nbtasm.hpp"
#include "../../nbtasm/backward.hpp"
#include "read_model.hpp"
#include "../../logicmachine/libnbt/constants.hpp"
#include "../../logicmachine/libnbt/collect_nanobots.hpp"

#include<string>
#include<vector>
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<stack>
#include<queue>
#include<cmath>
#include<algorithm>
#include<functional>
#include<list>
#include<deque>
#include<bitset>
#include<set>
#include<map>
#include<unordered_map>
#include<unordered_set>
#include<cstring>
#include<sstream>
#include<complex>
#include<iomanip>
#include<numeric>
#include<cassert>

using namespace std;

#define rep(X,Y) for (int (X) = 0;(X) < (Y);++(X))
#define reps(X,S,Y) for (int (X) = S;(X) < (Y);++(X))
#define rrep(X,Y) for (int (X) = (Y)-1;(X) >=0;--(X))
#define all(X) (X).begin(),(X).end()
#define pb push_back


#if DUMP_EN
#define dump(s) _dump(s)
#define dout(s) cout << #s << s << endl
#else
#define dump(s)
#define dout(s)
#endif

class Tree{
private:	
	map<Vec3,vector<Vec3>> originalTree;
	map<Vec3,set<Vec3>> tree;
	map<Vec3,Vec3> parent;
	set<Vec3> leaves;
public:
	Tree(VoxelGrid mat){
		int dx[]={1,0,0,-1, 0, 0};
		int dy[]={0,1,0, 0,-1, 0};
		int dz[]={0,0,1, 0, 0,-1};
		originalTree[Vec3{-1,-1,-1}];
		set<Vec3> visited;
		queue<Vec3> que;
		for(int x=0; x<mat.r(); ++x){
			for(int z=0; z<mat.r(); ++z){
				if(mat(z,0,x)){
					que.push(Vec3{x,0,z});
					originalTree[Vec3{-1,-1,-1}].push_back(Vec3{x,0,z});
					visited.insert(Vec3{x,0,z});
					originalTree[Vec3{x,0,z}];
				}
			}
		}
		while(que.size()){
			Vec3 pos=que.front();
			que.pop();
			for(int d=0; d<6; ++d){
				int nx=pos.x+dx[d];
				int ny=pos.y+dy[d];
				int nz=pos.z+dz[d];
				if(nx<0 || mat.r()<=nx ||
					 ny<0 || mat.r()<=ny ||
					 nz<0 || mat.r()<=nz) continue;
				if(mat(nz,ny,nx) && visited.insert(Vec3{nx,ny,nz}).second){
					que.push(Vec3{nx,ny,nz});
					originalTree[pos].push_back(Vec3{nx,ny,nz});
					originalTree[Vec3{nx,ny,nz}];
				}
			}
		}
		for(auto &p:originalTree){
			if(p.second.size()){
				tree[p.first]=set<Vec3>(p.second.begin(), p.second.end());
			}else{
				leaves.emplace(p.first);
			}
			for(Vec3 &cld:p.second) parent[cld]=p.first;
		}
	}

	const set<Vec3>& getLeaves()const{ return leaves;}

	void eraseLeaf(Vec3 leaf){
		assert(leaves.count(leaf));
		Vec3 par=parent[leaf];
		tree[par].erase(leaf);
		if(tree[par].empty()) leaves.emplace(par);
		leaves.erase(leaf);
	}
};

struct ConflictInfo {
    set<Vec3> volatileMat;
    map<Vec3, int> botPos;
};
class MyState : public State{
public:
    ConflictInfo conflict_info;
    MyState(const VoxelGrid& initial, int max_num_bots) : State(initial, max_num_bots) { }
    void update(){
        conflict_info.volatileMat.clear();
        conflict_info.botPos.clear();
        rep(i, num_bots()) {
            conflict_info.botPos[bots(i).pos()] = i;
        }
    }
    void commit_w_update(){
        commit();
        update();
    }
};

void _dump(const MyState& s){
	for(size_t i = 0; i < s.num_bots(); ++i){
		std::cout << i << ": " << s.bots(i) << std::endl;
	}
	std::cout << std::endl;
}

int sign(int x){ return x > 0 ? 1 : (x < 0 ? -1 : 0);}
Vec3 unit_vec(Vec3 v) {
    int dim = 0;
    Vec3 ret;
    if (abs(v.x)) dim++, ret = Vec3{sign(v.x), 0, 0};
    if (abs(v.y)) dim++, ret = Vec3{0, sign(v.y), 0};
    if (abs(v.z)) dim++, ret = Vec3{0, 0, sign(v.z)};
    if (dim != 1) 
        assert(false);

    return ret;
}

const vector<Vec3> diffs_6 = {
    Vec3{1,0,0}, Vec3{-1,0,0}, Vec3{0,1,0},Vec3{0,-1,0},Vec3{0,0,1},Vec3{0,0,-1}
};
const vector<Vec3> diffs_nd = [](){
    vector<Vec3> ret;
    reps(dx, -1, 2) reps(dy, -1, 2) reps(dz, -1, 2) {
        Vec3 d = Vec3(dx,dy,dz);
        if (test_near_distance(d)) {
            ret.push_back(d);
        }
    }
    return ret;
}();

Vec3 hunt(MyState& s, Vec3 src, Tree& tree) {
    for (auto d : diffs_nd) {
        Vec3 n = src + d;
        if (!test_position_range(s.matrix().r(), n)) continue;
        if (s.conflict_info.volatileMat.count(n)) continue;
        if (s.conflict_info.botPos.count(n)) continue;
        if (tree.getLeaves().count(n)) {
            return n;
        }
    }
    return Vec3{-1,-1,-1};
}

queue<Vec3> leaf_path(MyState& s, Vec3 src, Tree& tree ) {
    set<Vec3> visited;
    queue<Vec3> q;
    map<Vec3, Vec3> prev;
    visited.insert(src);
    q.push(src);
    prev[src] = Vec3{-1,-1,-1};
    while(q.size()) {
        Vec3 cur = q.front(); q.pop();
        for (Vec3 d : diffs_6) {
            reps(len, 1, 15) {
                Vec3 nxt = cur + d * len;
                if (visited.count(nxt))
                    break;
                if (!test_position_range(s.matrix().r(), nxt))
                    break;
                // 衝突系
                if (s.matrix()(nxt.z, nxt.y, nxt.x))
                    break;
                if (s.conflict_info.volatileMat.count(nxt))
                    break;
                if (s.conflict_info.botPos.count(nxt))
                    break;
                
                visited.insert(nxt);
                q.push(nxt);
                prev[nxt] = cur;
                if (hunt(s, nxt, tree) != Vec3{-1,-1,-1}) {
                    vector<Vec3> ret;
                    Vec3 t = nxt;
                    while (t != Vec3{-1,-1,-1}) {
                        ret.push_back(t);
                        t = prev[t];
                    }
                    reverse(all(ret));
                    queue<Vec3> retQ;
                    rep(i, ret.size())
                        retQ.push(ret[i]);
                    return retQ;
                }
            }
        }
    }
    return {};
}

set<Vec3> smove_volatile(Vec3 src, Vec3 tar) {
    Vec3 ud = unit_vec(tar - src);
    set<Vec3> vol;
    for (Vec3 v = src; v != tar; v += ud) {
        vol.insert(v);
    }
    vol.insert(tar);

    return vol;
}

bool move_path_once(MyState& s, int botIx, queue<Vec3>& path) {
    if (path.size() <= 1) return false;
    if (path.front() != s.bots(botIx).pos()) assert(false);
    Vec3 cur = path.front(); path.pop();
    Vec3 d = path.front() -  cur;
    set<Vec3> vm = smove_volatile(cur, path.front());
    set<Vec3> t;
    set_intersection(all(s.conflict_info.volatileMat), all(vm), inserter(t,end(t)));
    if (t.size()) return false;
    
    s.conflict_info.volatileMat.insert(all(vm));
    s.bots(botIx).smove(d);

    return true;
}

// void move_path_all(MyState& s, int botIx, queue<Vec3>& path) {
//     while(path.size()){
//         move_path_once(s, botIx, path);
//         s.commit();
//         dump(s);
//     }
// }


bool try_fission(MyState& s, int botIx) {
    int seed_num = __builtin_popcountll(s.bots(botIx).seeds());
    if (seed_num == 0) {
        return false;
    }
    for (auto d : diffs_6) {
        Vec3 n = s.bots(botIx).pos() + d;
        if (s.matrix(n.z,n.y,n.x)) continue;
        if (s.conflict_info.botPos.count(n)) continue;
        if (s.conflict_info.volatileMat.count(n)) continue;
        if (!test_position_range(s.matrix().r(), n)) continue;

        s.bots(botIx).fission(d, seed_num/2);
        s.conflict_info.volatileMat.insert(n);
        return true;
    }
    return false;
}
bool try_hunt(MyState& s, int botIx, Tree& tree) {
    Vec3 box = hunt(s, s.bots(botIx).pos(), tree);
    if (box != Vec3{-1,-1,-1}) {
        Vec3 d = box - s.bots(botIx).pos();
        s.bots(botIx).empty(d);
        tree.eraseLeaf(box);
        return true;
    }
    return false;
}
bool try_move2leaf(MyState& s, int botIx, Tree& tree) {
    auto path = leaf_path(s, s.bots(botIx).pos(), tree);
    if (path.size() == 0) {
        return false;
    }
    return move_path_once(s, botIx, path);
}

void solve(MyState& s) {
    Tree tree(s.matrix());
    set<Vec3> volatileMat;
    
    
    s.update();
    dump(s);
    
    while(tree.getLeaves().begin()->x>=0) {
        rep(botIx, s.num_bots()) {
            if (try_fission(s, botIx));
            else if (try_hunt(s, botIx, tree));
            else if (try_move2leaf(s,botIx, tree));
            else ;
        }
        s.commit_w_update();
        dump(s);
    }
#if 0
    while(1){
		vector<int> todo;
		set<Vec3> ng;
		rep(i,s.num_bots()){
			if(s.bots(i).pos().x) todo.pb(i);
			ng.insert(s.bots(i).pos());
		}
		if(todo.empty()) break;
		for(int i:todo){
			auto p=s.bots(i).pos();
			rep(j,15){
				if(ng.count(Vec3{p.x-1,p.y,p.z})) break;
				--p.x;
				if(p.x==0) break;
			}
			s.bots(i).smove(p-s.bots(i).pos());
		}
		s.commit_w_update();
        dump(s);
		vector<int> rem(s.num_bots()); iota(all(rem),0);
		rep(i,rem.size())rep(j,i)if(test_near_distance(s.bots(rem[i]).pos()-s.bots(rem[j]).pos())){
			int u=rem[i],v=rem[j];
			s.bots(u).fusion_s(s.bots(v).pos()-s.bots(u).pos());
			s.bots(v).fusion_p(s.bots(u).pos()-s.bots(v).pos());
			rem.erase(rem.begin()+i);
			goto BRK2;
		}
BRK2:
		s.commit_w_update();
        dump(s);
	}
	dump(s);

	while(1){
		vector<int> todo;
		set<Vec3> ng;
		rep(i,s.num_bots()){
			if(s.bots(i).pos().y) todo.pb(i);
			ng.insert(s.bots(i).pos());
		}
		if(todo.empty()) break;
		for(int i:todo){
			auto p=s.bots(i).pos();
			rep(j,15){
				if(ng.count(Vec3{p.x,p.y-1,p.z})) break;
				--p.y;
				if(p.y==0) break;
			}
			s.bots(i).smove(p-s.bots(i).pos());
		}
		s.commit_w_update();
        dump(s);
		vector<int> rem(s.num_bots()); iota(all(rem),0);
		rep(i,rem.size())rep(j,i)if(test_near_distance(s.bots(rem[i]).pos()-s.bots(rem[j]).pos())){
			int u=rem[i],v=rem[j];
			s.bots(u).fusion_s(s.bots(v).pos()-s.bots(u).pos());
			s.bots(v).fusion_p(s.bots(u).pos()-s.bots(v).pos());
			rem.erase(rem.begin()+i);
			goto BRK3;
		}
BRK3:
		s.commit_w_update();
        dump(s);
	}
	dump(s);

	while(1){
		vector<int> todo;
		set<Vec3> ng;
		rep(i,s.num_bots()){
			if(s.bots(i).pos().z) todo.pb(i);
			ng.insert(s.bots(i).pos());
		}
		if(todo.empty()) break;
		for(int i:todo){
			auto p=s.bots(i).pos();
			rep(j,15){
				if(ng.count(Vec3{p.x,p.y,p.z-1})) break;
				--p.z;
				if(p.z==0) break;
			}
			s.bots(i).smove(p-s.bots(i).pos());
		}
		s.commit_w_update();
        dump(s);
		vector<int> rem(s.num_bots()); iota(all(rem),0);
		rep(i,rem.size())rep(j,i)if(test_near_distance(s.bots(rem[i]).pos()-s.bots(rem[j]).pos())){
			int u=rem[i],v=rem[j];
			s.bots(u).fusion_s(s.bots(v).pos()-s.bots(u).pos());
			s.bots(v).fusion_p(s.bots(u).pos()-s.bots(v).pos());
			rem.erase(rem.begin()+i);
			goto BRK;
		}
BRK:
//		s.dump_pending_commands(std::cerr);
		s.commit_w_update();
        dump(s);
	}
#endif
    collect_nanobots_x(s);
    collect_nanobots_y(s);
    collect_nanobots_z(s);

	dump(s);

    s.bots(0).halt();
    s.commit_w_update();
    dump(s);
}

int main(int argc, char const *argv[])
{
    VoxelGrid model = read_data(argv[1]);
    MyState s(model, 40);

    solve(s);

    // output nbt
	string file(argv[1]);
	{
		int n=file.size();
		rrep(i,n)if(file[i]=='/'){
			file=file.substr(i+1);
			break;
		}
	}
    export_backward_trace((file.substr(0,5)+".nbt").c_str(), s);
    
    return 0;
}
