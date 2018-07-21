#include "../nbtasm/nbtasm.hpp"
#include "../nbtasm/backward.hpp"
#include "read_model.hpp"
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
#define X first
#define Y second
#define pb push_back
#define rep(X,Y) for (int (X) = 0;(X) < (Y);++(X))
#define reps(X,S,Y) for (int (X) = S;(X) < (Y);++(X))
#define rrep(X,Y) for (int (X) = (Y)-1;(X) >=0;--(X))
#define rreps(X,S,Y) for (int (X) = (Y)-1;(X) >= (S);--(X))
#define repe(X,Y) for ((X) = 0;(X) < (Y);++(X))
#define peat(X,Y) for (;(X) < (Y);++(X))
#define all(X) (X).begin(),(X).end()
#define rall(X) (X).rbegin(),(X).rend()
#define eb emplace_back
#define UNIQUE(X) (X).erase(unique(all(X)),(X).end())
#define Endl endl

using namespace std;
typedef long long ll;
typedef pair<int,int> pii;
typedef pair<ll,ll> pll;
template<class T> using vv=vector<vector<T>>;
template<class T> ostream& operator<<(ostream &os, const vector<T> &t) {
os<<"{"; rep(i,t.size()) {os<<t[i]<<",";} os<<"}"<<endl; return os;}
template<class T,size_t n> ostream& operator<<(ostream &os, const array<T,n> &t) {
	os<<"{"; rep(i,n) {os<<t[i]<<",";} os<<"}"<<endl; return os;}
template<class S, class T> ostream& operator<<(ostream &os, const pair<S,T> &t) { return os<<"("<<t.first<<","<<t.second<<")";}
template<class S, class T,class U> ostream& operator<<(ostream &os, const tuple<S,T,U> &t) { return os<<"("<<get<0>(t)<<","<<get<1>(t)<<","<<get<2>(t)<<")";}
template<class S, class T,class U,class V> ostream& operator<<(ostream &os, const tuple<S,T,U,V> &t) { return os<<"("<<get<0>(t)<<","<<get<1>(t)<<","<<get<2>(t)<<","<<get<3>(t)<<")";}
template<class S, class T,class U,class V,class W> ostream& operator<<(ostream &os, const tuple<S,T,U,V,W> &t) { return os<<"("<<get<0>(t)<<","<<get<1>(t)<<","<<get<2>(t)<<","<<get<3>(t)<<","<<get<4>(t)<<")";}
template<class T> inline bool MX(T &l,const T &r){return l<r?l=r,1:0;}
template<class T> inline bool MN(T &l,const T &r){return l>r?l=r,1:0;}
//#undef NUIP
#ifdef NUIP
#define out(args...){vector<string> a_r_g_s=s_p_l_i_t(#args, ','); e_r_r(a_r_g_s.begin(), args); }
vector<string> s_p_l_i_t(const string &s, char c){vector<string> v;int d=0,f=0;string t;for(char c:s){if(!d&&c==',')v.pb(t),t="";else t+=c;if(c=='\"'||c=='\'')f^=1;if(!f&&c=='(')++d;if(!f&&c==')')--d;}v.pb(t);return move(v);}
void e_r_r(vector<string>::iterator it) {}
template<typename T, typename... Args> void e_r_r(vector<string>::iterator it, T a, Args... args){ if(*it==" 1"||*it=="1") cerr<<endl; else cerr << it -> substr((*it)[0] == ' ', it -> length()) << " = " << a << ", "; e_r_r(++it, args...);}
#else
#define out
#endif
#ifdef __cpp_init_captures
template<typename T>vector<T> table(int n, T v){ return vector<T>(n, v);}
template <class... Args> auto table(int n, Args... args){auto val = table(args...); return vector<decltype(val)>(n, move(val));}
#endif
const ll MOD=1e9+7;

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

void dump(const State& s){
	for(size_t i = 0; i < s.num_bots(); ++i){
		std::cout << i << ": " << s.bots(i) << std::endl;
	}
	std::cout << std::endl;
}

Vec3 hunt(const Tree &v,const Vec3 p){
	reps(dx,-1,2)reps(dy,-1,2)reps(dz,-1,2)if(abs(dx)+abs(dy)+abs(dz)<3){
		int nx=p.x+dx;
		int ny=p.y+dy;
		int nz=p.z+dz;
		if(v.getLeaves().count(Vec3{nx,ny,nz})) return Vec3{dx,dy,dz};
	}
	return Vec3{0,0,0};
}

vector<Vec3> move(const Tree &g,const VoxelGrid &v,const Vec3 p){
	static int dx[]={1,0,0,-1, 0, 0};
	static int dy[]={0,1,0, 0,-1, 0};
	static int dz[]={0,0,1, 0, 0,-1};
	queue<Vec3> que; que.push(p);
	set<Vec3> vst; vst.insert(p);
	map<Vec3,Vec3> prv; prv[p]=Vec3{-1,-1,-1};
	while(que.size()){
		Vec3 p=que.front();
		rep(d,6){
			int nx=p.x, ny=p.y, nz=p.z;
			rep(i,15){
				nx+=dx[d]; ny+=dy[d]; nz+=dz[d];
				if(nx<0 || v.r()<=nx ||
					 ny<0 || v.r()<=ny ||
					 nz<0 || v.r()<=nz) break;
				if(v(nz,ny,nx)) break;
				Vec3 nxt{nx,ny,nz};
				if(vst.emplace(nxt).Y){
					prv[nxt]=p;
					que.push(nxt);
					if(hunt(g,nxt)!=Vec3{0,0,0}){
						vector<Vec3> re;
						for(Vec3 cur=nxt; cur.x>=0; cur=prv[cur]) re.pb(cur);
						reverse(all(re));
						return re;
					}
				}
			}
		}
	}
	assert(0);
}

int main(int argc, char* argv[]){

	VoxelGrid v = read_data(argv[1]);
	const int R=v.r();
	
	State s(v, 20);
	Tree g(v);
	while(g.getLeaves().begin()->x>=0){
		for(auto p:g.getLeaves()) cout<<p<<",";cout<<endl;
		Vec3 tar=hunt(g,s.bots(0).pos());
		if(tar==Vec3{0,0,0}){
			auto vec=move(g,s.matrix(),s.bots(0).pos());
			reps(i,1,vec.size()){
				s.bots(0).smove(vec[i]-vec[i-1]);
				s.commit();
			}
			tar=hunt(g,vec.back());
			out(vec,s.bots(0).pos(),tar,1);
		}
		s.bots(0).empty(tar);
		out("eat",s.bots(0).pos()+tar,1);
		s.commit();
		g.eraseLeaf(s.bots(0).pos()+tar);
	}
	dump(s);

	while(s.bots(0).pos().x){
		s.bots(0).smove(Vec3{-min(15,s.bots(0).pos().x),0,0});
		s.commit();
	}
	dump(s);
	while(s.bots(0).pos().y){
		s.bots(0).smove(Vec3{0,-min(15,s.bots(0).pos().y),0});
		s.commit();
	}
	dump(s);
	while(s.bots(0).pos().z){
		s.bots(0).smove(Vec3{0,0,-min(15,s.bots(0).pos().z)});
		s.commit();
	}
	dump(s);

	s.bots(0).halt();
	s.commit();
	dump(s);
	
	export_backward_trace("trace.nbt", s);
return 0;
}
