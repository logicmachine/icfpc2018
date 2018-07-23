#define STRONG_VALIDATION
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
	// for(size_t i = 0; i < s.num_bots(); ++i){
	// 	std::cout << i << ": " << s.bots(i) << std::endl;
	// }
	// std::cout << std::endl;
}

Vec3 hunt(const Tree &v,const set<Vec3> ban,const Vec3 p){
	reps(dx,-1,2)reps(dy,-1,2)reps(dz,-1,2)if(abs(dx)+abs(dy)+abs(dz)<3){
		int nx=p.x+dx;
		int ny=p.y+dy;
		int nz=p.z+dz;
		Vec3 tmp{nx,ny,nz};
		if(v.getLeaves().count(tmp) && !ban.count(tmp)) return Vec3{dx,dy,dz};
	}
	return Vec3{0,0,0};
}

vector<Vec3> move(const Tree &g,const VoxelGrid &v,const set<Vec3> &ban,const Vec3 p){
	if(g.getLeaves().begin()->x<0) return {};
	static int dx[]={1,0,0,-1, 0, 0};
	static int dy[]={0,1,0, 0,-1, 0};
	static int dz[]={0,0,1, 0, 0,-1};
	queue<Vec3> que; que.push(p);
	set<Vec3> vst; vst.insert(p);
	map<Vec3,Vec3> prv; prv[p]=Vec3{-1,-1,-1};
	while(que.size()){
		//out(que.size(),v.r(),1);
		Vec3 p=que.front();
		que.pop();
		// vector<int> ds(6); iota(all(ds),0); random_shuffle(all(ds));
		// for(auto d:ds){
		rep(d,6){
			int nx=p.x, ny=p.y, nz=p.z;
			rep(i,15){
				nx+=dx[d]; ny+=dy[d]; nz+=dz[d];
				if(nx<0 || v.r()<=nx ||
					 ny<0 || v.r()<=ny ||
					 nz<0 || v.r()<=nz) break;
				Vec3 nxt{nx,ny,nz};
				if(v(nz,ny,nx) || ban.count(nxt)) break;
				if(vst.emplace(nxt).Y){
					prv[nxt]=p;
					que.push(nxt);
					if(hunt(g,ban,nxt)!=Vec3{0,0,0}){
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

int main(int argc, char* argv[]){

	VoxelGrid v = read_data(argv[1]);
	const int R=v.r();

	State s(v, 20);

	if(R<=2){
		s.bots(0).halt();
		s.commit();
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
	
	const int N=4*min(5,R-1);
	{
		const int dif=(R-1)/(N/4);
		auto straight=[&](Vec3 dst){
										while(s.bots(0).pos()!=dst){
											auto d=dst-s.bots(0).pos();
											MN(d.x,15); MX(d.x,-15);
											MN(d.y,15); MX(d.y,-15);
											MN(d.z,15); MX(d.z,-15);
											s.bots(0).smove(d);
											s.commit();
										}
									};
		rep(i,N/4){
			straight(s.bots(0).pos() + Vec3{dif,0,0});
			s.bots(0).fission(Vec3{-1, 0, 0}, 0);
			s.commit();
		}
		straight(Vec3{R-1,0,0});
		dump(s);

		rep(i,N/4){
			straight(s.bots(0).pos() + Vec3{0,0,dif});
			s.bots(0).fission(Vec3{ 0, 0,-1}, 0);
			s.commit();
		}
		straight(Vec3{R-1,0,R-1});
		dump(s);

		rep(i,N/4){
			straight(s.bots(0).pos() + Vec3{-dif,0,0});
			s.bots(0).fission(Vec3{ 1, 0, 0}, 0);
			s.commit();
		}
		straight(Vec3{0,0,R-1});
		dump(s);

		rep(i,N/4){
			straight(s.bots(0).pos() + Vec3{0,0,-dif});
		dump(s);
			if(i+1==N/4) break;
			s.bots(0).fission(Vec3{ 0, 0, 1}, 0);
			s.commit();
		}
		straight(Vec3{0,0,0});
		dump(s);
	}

	Tree g(v);
	ll bad=0;
	while(g.getLeaves().begin()->x>=0){
		//for(auto p:g.getLeaves()) cout<<p<<",";cout<<endl;
		set<Vec3> ban;
		auto banInt=[&ban](Vec3 a,Vec3 b){
									//out(a,b,1);
									reps(x,min(a.x,b.x),max(a.x,b.x)+1)
									reps(y,min(a.y,b.y),max(a.y,b.y)+1)
									reps(z,min(a.z,b.z),max(a.z,b.z)+1)
										ban.insert(Vec3{x,y,z});
								};
		rep(i,N) ban.insert(s.bots(i).pos());
		rep(i,N){
			Vec3 tar=hunt(g,ban,s.bots(i).pos());
			//out(tar,1);
			if(tar!=Vec3{0,0,0}){
				auto tmp=s.bots(i).pos()+tar;
				//out("eat",i,s.bots(i).pos()+tar,ban.count(tmp),v(tmp.z,tmp.y,tmp.x)==1,s.matrix(tmp.z,tmp.y,tmp.x)==1,1);
				s.bots(i).empty(tar); ban.emplace(s.bots(i).pos()+tar);
				g.eraseLeaf(s.bots(i).pos()+tar);
				continue;
			}else{
				auto tmp=move(g,s.matrix(),ban,s.bots(i).pos());
				if(tmp.empty()) continue;
				if(tmp.size()>10){
					++bad;
					reps(i,1,tmp.size()){
						s.bots(i).smove(tmp[i]-tmp[i-1]);
				//		s.dump_pending_commands(std::cerr);
						s.commit();
					}
					ban.clear();
				}else{
					s.bots(i).smove(tmp[1]-tmp[0]);
					banInt(tmp[1],tmp[0]);
				}
			}
		}
//		s.dump_pending_commands(std::cerr);
		s.commit();
		out(g.getLeaves().size(),1);
	}
	dump(s);
	out(2,1);

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
		s.commit();
		vector<int> rem(s.num_bots()); iota(all(rem),0);
		rep(i,rem.size())rep(j,i)if(test_near_distance(s.bots(rem[i]).pos()-s.bots(rem[j]).pos())){
			int u=rem[i],v=rem[j];
			s.bots(u).fusion_s(s.bots(v).pos()-s.bots(u).pos());
			s.bots(v).fusion_p(s.bots(u).pos()-s.bots(v).pos());
			rem.erase(rem.begin()+i);
			goto BRK2;
		}
BRK2:
		s.commit();
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
		s.commit();
		vector<int> rem(s.num_bots()); iota(all(rem),0);
		rep(i,rem.size())rep(j,i)if(test_near_distance(s.bots(rem[i]).pos()-s.bots(rem[j]).pos())){
			int u=rem[i],v=rem[j];
			s.bots(u).fusion_s(s.bots(v).pos()-s.bots(u).pos());
			s.bots(v).fusion_p(s.bots(u).pos()-s.bots(v).pos());
			rem.erase(rem.begin()+i);
			goto BRK3;
		}
BRK3:
		s.commit();
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
		s.commit();
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
		s.commit();
	}
	dump(s);

	s.bots(0).halt();
	//s.dump_pending_commands(std::cerr);
	s.commit();
	dump(s);

	// cout<<"hoge"<<endl;

	string file(argv[1]);
	{
		int n=file.size();
		rrep(i,n)if(file[i]=='/'){
			file=file.substr(i+1);
			break;
		}
	}
	//export_backward_trace((file.substr(0,5)+".nbt").c_str(), s);
	s.export_trace((file.substr(0,5)+".nbt").c_str());
	return 0;
}
