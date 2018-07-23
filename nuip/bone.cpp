// #define STRONG_VALIDATION
#include "../nbtasm/nbtasm.hpp"
#include "../nbtasm/backward.hpp"
#include "read_model.hpp"
#include "GtreeS.hpp"
#include <string>
#include <vector>
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

void dump(const State& s){
#ifdef NUIP
	for(size_t i = 0; i < s.num_bots(); ++i){
		std::cout << i << ": " << s.bots(i) << std::endl;
	}
	std::cout << std::endl;
#endif
}
int SUGOIOP=0;
class MyState:public State{
	set<Vec3> ng;
	const int dx[6]={1,0,0,-1, 0, 0};
	const int dy[6]={0,1,0, 0,-1, 0};
	const int dz[6]={0,0,1, 0, 0,-1};
public:
	GtreeS g;
	MyState(VoxelGrid v,int n):State(v,n),g(v){}
	MyState(int R,VoxelGrid v,int n):State(R,n),g(v){}
	void home(){
		// do{
		// 	shake(1);
		// 	initBan();
		// }while(!ng.count(Vec3{0,0,0}));
		rep(i,num_bots()) bots(i).wait();
		initBan();
		auto tmp=move(Vec3{-1,0,-1},bots(0).pos());
		reps(i,1,tmp.size()){
			bots(0).smove(tmp[i]-tmp[i-1]);
			commit();
		}
		while(num_bots()>1){
			auto tmp=move(Vec3{0,0,0},bots(1).pos());
			reps(i,1,tmp.size()){
				bots(1).smove(tmp[i]-tmp[i-1]);
				commit();
			}
			bots(0).fusion_p(bots(1).pos());
			bots(1).fusion_s(-bots(1).pos());
			commit();
		}
	}
	vector<Vec3> runaway(Vec3 src){
		if(!ng.count(src)) return {src};
		rep(d,6){
			int nx=src.x, ny=src.y, nz=src.z;
			rep(i,15){
				nx+=dx[d]; ny+=dy[d]; nz+=dz[d];
				Vec3 nxt{nx,ny,nz};
				if(!nxt.region_check(matrix().r())) break;
				if(matrix()(nz,ny,nx) || ng.count(nxt)) break;
				return {src,nxt};
			}
		}
		// while(que.size()){
		// 	Vec3 p=que.front();
		// 	que.pop();
		// 	rep(d,6){
		// 		int nx=p.x, ny=p.y, nz=p.z;
		// 		rep(i,15){
		// 			nx+=dx[d]; ny+=dy[d]; nz+=dz[d];
		// 			Vec3 nxt{nx,ny,nz};
		// 			if(!nxt.region_check(matrix().r())) break;
		// 			if(matrix()(nz,ny,nx) || ng.count(nxt)) break;
		// 			return {src,nxt};
		// 		}
		// 	}
		// }
		return {};
	}
	vector<Vec3> move(Vec3 dst,Vec3 src){
		if(test_near_distance(dst-src) && !ng.count(src)) return {src};
		queue<Vec3> que; que.push(src);
		set<Vec3> vst; vst.insert(src);
		map<Vec3,Vec3> prv; prv[src]=Vec3{-1,-1,-1};
		while(que.size()){
			Vec3 p=que.front();
			que.pop();
			rep(d,6){
				int nx=p.x, ny=p.y, nz=p.z;
				rep(i,15){
					nx+=dx[d]; ny+=dy[d]; nz+=dz[d];
					Vec3 nxt{nx,ny,nz};
					if(!nxt.region_check(matrix().r())) break;
					if(matrix()(nz,ny,nx) || ng.count(nxt)) break;
					if(vst.emplace(nxt).Y){
						prv[nxt]=p;
						que.push(nxt);
						if(test_near_distance(dst-nxt)){
							vector<Vec3> re;
							for(Vec3 cur=nxt; cur.x>=0; cur=prv[cur]) re.pb(cur);
							reverse(all(re));
							return re;
						}
					}
				}
			}
		}
		return {};
	}	
	bool reachable(Vec3 dst,Vec3 src){
		return move(dst,src).size();
	}
	bool reachable(pair<Vec3,Vec3> box,Vec3 src){
		out("reachable",box,1);
		Vec3 l,r; tie(l,r)=box;
		if(!reachable(Vec3{l.x,l.y,l.z},src)) return 0;
		if(!reachable(Vec3{l.x,l.y,r.z},src)) return 0;
		if(!reachable(Vec3{l.x,r.y,l.z},src)) return 0;
		if(!reachable(Vec3{l.x,r.y,r.z},src)) return 0;
		if(!reachable(Vec3{r.x,l.y,l.z},src)) return 0;
		if(!reachable(Vec3{r.x,l.y,r.z},src)) return 0;
		if(!reachable(Vec3{r.x,r.y,l.z},src)) return 0;
		if(!reachable(Vec3{r.x,r.y,r.z},src)) return 0;
		return 1;
	}
	bool reachableB(Vec3 dst,Vec3 src){
		return reachable(make_pair(dst,getRB(dst)),src);
	}
	Vec3 getRB(const Vec3 p){
		auto it=g.getRoots().lower_bound(make_pair(p,p));
		if(it!=g.getRoots().end() && it->X==p) return it->Y;
		return Vec3{-1,-1,-1};
	}
	bool contains(Vec3 p,Vec3 a,Vec3 b){ //p in [a,b]?
		if(p.x < min(a.x,b.x) || p.x > max(a.x,b.x)) return 0;
		if(p.y < min(a.y,b.y) || p.y > max(a.y,b.y)) return 0;
		if(p.z < min(a.z,b.z) || p.z > max(a.z,b.z)) return 0;
		return 1;
	}
	Vec3 hunt(const Vec3 p){
		reps(dx,-1,2)reps(dy,-1,2)reps(dz,-1,2)if(abs(dx)+abs(dy)+abs(dz)<3){
			int nx=p.x+dx;
			int ny=p.y+dy;
			int nz=p.z+dz;
			Vec3 LB{nx,ny,nz};
			Vec3 RB=getRB(LB);
			if(SUGOIOP && p==Vec3{6,0,11}) out(LB,RB,1);
			if(RB.x>=0  && !ng.count(p) && !contains(p,LB,RB)){
				return Vec3{dx,dy,dz};
			}
		}
		return Vec3{0,0,0};
	}
	vector<Vec3> getBox(int bot){ return getBox(bots(bot).pos());}
	vector<Vec3> getBox(Vec3 pos){
		if(g.getRoots().empty()) return {};
		queue<Vec3> que; que.push(pos);
		set<Vec3> vst; vst.insert(pos);
		map<Vec3,Vec3> prv; prv[pos]=Vec3{-1,-1,-1};
		while(que.size()){
			Vec3 p=que.front();
			que.pop();
			rep(d,6){
				int nx=p.x, ny=p.y, nz=p.z;
				rep(i,15){
					nx+=dx[d]; ny+=dy[d]; nz+=dz[d];
					Vec3 nxt{nx,ny,nz};
					if(!nxt.region_check(matrix().r())) break;
					if(matrix()(nz,ny,nx) || ng.count(nxt)) break;
					if(vst.emplace(nxt).Y){
						prv[nxt]=p;
						que.push(nxt);
						auto d=hunt(nxt);
						if(d!=Vec3{0,0,0} && reachableB(nxt+d,nxt)){
							vector<Vec3> re;
							for(Vec3 cur=nxt; cur.x>=0; cur=prv[cur]) re.pb(cur);
							reverse(all(re));
							return re;
						}
					}
				}
			}
		}
		return {};
		assert(0);
	}
	void ban(Vec3 a){
		ng.insert(a);
	}
	void ban(Vec3 a,Vec3 b){
		reps(z,min(a.z,b.z),max(a.z,b.z)+1)
			reps(y,min(a.y,b.y),max(a.y,b.y)+1)
			reps(x,min(a.x,b.x),max(a.x,b.x)+1) ban(Vec3{x,y,z});
	}
	void initBan(){
		ng.clear();
		rep(i,num_bots()) ng.insert(bots(i).pos());
	}
	void shake(int t=20){
		rep(_,t){
			initBan();
		rep(i,num_bots()){
			Vec3 p=bots(i).pos();
			vector<int> ds(6); iota(all(ds),0); random_shuffle(all(ds));
			int mx=0;
			Vec3 dst;
			for(int d:ds){
				int nx=p.x, ny=p.y, nz=p.z;
				rep(i,15){
					nx+=dx[d]; ny+=dy[d]; nz+=dz[d];
					Vec3 nxt{nx,ny,nz};
					if(!nxt.region_check(matrix().r())) break;
					if(matrix()(nz,ny,nx) || ng.count(nxt)) break;
					if(MX(mx,i)) dst=Vec3{nx,ny,nz};
				}
			}
			if(mx){
				ban(p,dst);
				//out(p,dst,1);
				bots(i).smove(dst-p);
			}
		}
		commit();
		}
		out("shaked",1);
		dump(*this);
	}

	void terminate(){

		while(1){
			vector<int> todo;
			set<Vec3> ng;
			rep(i,num_bots()){
				if(bots(i).pos().x) todo.pb(i);
				ng.insert(bots(i).pos());
			}
			if(todo.empty()) break;
			for(int i:todo){
				auto p=bots(i).pos();
				rep(j,15){
					if(ng.count(Vec3{p.x-1,p.y,p.z})) break;
					--p.x;
					if(p.x==0) break;
				}
				bots(i).smove(p-bots(i).pos());
			}
			commit();
			vector<int> rem(num_bots()); iota(all(rem),0);
			rep(i,rem.size())rep(j,i)if(test_near_distance(bots(rem[i]).pos()-bots(rem[j]).pos())){
				int u=rem[i],v=rem[j];
				bots(u).fusion_s(bots(v).pos()-bots(u).pos());
				bots(v).fusion_p(bots(u).pos()-bots(v).pos());
				rem.erase(rem.begin()+i);
				goto BRK2;
			}
		BRK2:
			commit();
		}

		while(1){
			vector<int> todo;
			set<Vec3> ng;
			rep(i,num_bots()){
				if(bots(i).pos().y) todo.pb(i);
				ng.insert(bots(i).pos());
			}
			if(todo.empty()) break;
			for(int i:todo){
				auto p=bots(i).pos();
				rep(j,15){
					if(ng.count(Vec3{p.x,p.y-1,p.z})) break;
					--p.y;
					if(p.y==0) break;
				}
				bots(i).smove(p-bots(i).pos());
			}
			commit();
			vector<int> rem(num_bots()); iota(all(rem),0);
			rep(i,rem.size())rep(j,i)if(test_near_distance(bots(rem[i]).pos()-bots(rem[j]).pos())){
				int u=rem[i],v=rem[j];
				bots(u).fusion_s(bots(v).pos()-bots(u).pos());
				bots(v).fusion_p(bots(u).pos()-bots(v).pos());
				rem.erase(rem.begin()+i);
				goto BRK3;
			}
		BRK3:
			commit();
		}

		while(1){
			vector<int> todo;
			set<Vec3> ng;
			rep(i,num_bots()){
				if(bots(i).pos().z) todo.pb(i);
				ng.insert(bots(i).pos());
			}
			if(todo.empty()) break;
			for(int i:todo){
				auto p=bots(i).pos();
				rep(j,15){
					if(ng.count(Vec3{p.x,p.y,p.z-1})) break;
					--p.z;
					if(p.z==0) break;
				}
				bots(i).smove(p-bots(i).pos());
			}
			commit();
			vector<int> rem(num_bots()); iota(all(rem),0);
			rep(i,rem.size())rep(j,i)if(test_near_distance(bots(rem[i]).pos()-bots(rem[j]).pos())){
				int u=rem[i],v=rem[j];
				bots(u).fusion_s(bots(v).pos()-bots(u).pos());
				bots(v).fusion_p(bots(u).pos()-bots(v).pos());
				rem.erase(rem.begin()+i);
				goto BRK;
			}
		BRK:
			commit();
		}

		bots(0).halt();
		commit();
	}
};

int main(int argc, char* argv[]){

	VoxelGrid v = read_data(argv[1]);
	const int R=v.r();
	
	MyState s(R, v, 40);

	assert(R>=8);
	s.bots(0).fission(Vec3{ 0, 0, 1}, 0); s.commit();
	rep(i,6){
		s.bots(0).smove(Vec3{ 1, 0, 0}); s.commit();
		s.bots(0).fission(Vec3{-1, 0, 0}, 0); s.commit();
	}
	dump(s);

	auto giveUp=[&](){
								string file(argv[1]);
								int n=file.size();
								rrep(i,n)if(file[i]=='/'){
									file=file.substr(i+1);
									break;
								}
								s.home();
								s.bots(0).halt(); s.commit();
								out("GIVE UP",file,1);
								s.export_trace((file+"BONE.nbt").c_str());
								write_data((file+"BONE.mdl").c_str(),s.matrix());
								exit(0);
							};

	try{
	while(s.g.getRoots().size()){
		s.initBan();
		vector<int> id(8); iota(all(id),0); random_shuffle(all(id));
		auto vec=s.getBox(id[0]);
		if(vec.size()==0){
			out("box not found",1);
			s.shake();
			if(SUGOIOP){
				giveUp();
			}
			SUGOIOP=1;
			if(rand()%5) continue;
			giveUp();
		}
		out(vec,1);
		auto LB=vec.back()+s.hunt(vec.back());
		auto RB=s.getRB(LB);
		// if(LB==Vec3{20,19,20}){
		// 				s.export_trace("trace.nbt");
		// 	for(auto p:s.g.getLeaves()) cout<<p;cout<<endl;
		// 	return 0;
		// }
		s.ban(LB,RB);
		out(LB,RB,1);
		vector<Vec3> vs;
		vs.pb(Vec3{LB.x, LB.y, LB.z});
		vs.pb(Vec3{LB.x, LB.y, RB.z});
		vs.pb(Vec3{LB.x, RB.y, LB.z});
		vs.pb(Vec3{LB.x, RB.y, RB.z});
		vs.pb(Vec3{RB.x, LB.y, LB.z});
		vs.pb(Vec3{RB.x, LB.y, RB.z});
		vs.pb(Vec3{RB.x, RB.y, LB.z});
		vs.pb(Vec3{RB.x, RB.y, RB.z});
		sort(all(vs)); UNIQUE(vs);
		rrep(timeBox,10){
			s.initBan(); s.ban(LB,RB);
			vv<Vec3> way(8);
			int maxSize=0;
			int minSize=MOD;
			rrep(i,8){
				if(i<vs.size()){
					way[i]=s.move(vs[i],s.bots(id[i]).pos());
				}else{
					way[i]=s.runaway(s.bots(id[i]).pos());
				}
				MX<int>(maxSize, way[i].size());
				MN<int>(minSize, way[i].size());
				reps(j,1,way[i].size()) s.ban(way[i][j-1],way[i][j]);
			}
			if(maxSize==1 && minSize==0){
				out(way,1);
				out("can't move",1);
				out(LB,RB,1);
				SUGOIOP=1;
				// s.shake();
				if(rand()%4){
					s.shake(); s.ban(LB,RB);
					random_shuffle(all(id));
					continue;
				}
				dump(s);
				giveUp();
			}
			out(way,1);
			reps(j,1,10){
				int upd=0;
				rep(k,vs.size())if(j<way[k].size()){
					s.bots(id[k]).smove(way[k][j]-way[k][j-1]);
					upd=1;
				}
				if(!upd) break;
				s.commit();
			}
			int ready=1;
			rep(i,vs.size())if(!test_near_distance(vs[i]-s.bots(id[i]).pos())) ready=0;
			if(ready) break;
			if(timeBox==0){
				giveUp();
			}
		}
		dump(s);
		int t=vs.size();
		if(vs.size()==2) rep(i,2) vs.pb(vs[i]);
		if(vs.size()==4) rep(i,4) vs.pb(vs[i]);
		if(t==1){
			s.bots(id[0]).fill(vs[0]-s.bots(id[0]).pos());			
		}else{
			rep(i,t){
				assert(test_near_distance(vs[i]-s.bots(id[i]).pos()));
				s.bots(id[i]).gfill(vs[i]-s.bots(id[i]).pos(), vs[i^7]-vs[i]);
			}
		}
		s.g.eraseRoot(make_pair(LB,RB));
		s.commit();
		dump(s);
		//return 0;
	}
	}catch (CommandError &ababa){
		giveUp();
	}

	string file(argv[1]);
	{
		int n=file.size();
		rrep(i,n)if(file[i]=='/'){
			file=file.substr(i+1);
			break;
		}
	}

	//s.terminate();
	giveUp();

	out("DONE",1);

	s.export_trace((file.substr(0,5)+".nbt").c_str());
	
	return 0;
}

