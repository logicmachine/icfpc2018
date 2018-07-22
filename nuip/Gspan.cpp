#include<bits/stdc++.h>
#include "nbtasm.hpp"
#include "read_model.hpp"

using namespace std;

/*
 有向全域木を構築
 ground は Vec3{-1, -1, -1}
 */

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

map<Vec3,vector<Vec3>> span(VoxelGrid mat){
	int dx[]={1,0,0,-1, 0, 0};
	int dy[]={0,1,0, 0,-1, 0};
	int dz[]={0,0,1, 0, 0,-1};
	map<Vec3,vector<Vec3>> re;
	set<Vec3> visited;
	queue<Vec3> que;
	for(int x=0; x<mat.r(); ++x){
		for(int z=0; z<mat.r(); ++z){
			if(mat(z,0,x)){
				que.push(Vec3{x,0,z});
				re[Vec3{-1,-1,-1}].push_back(Vec3{x,0,z});
				visited.insert(Vec3{x,0,z});
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
				re[pos].push_back(Vec3{nx,ny,nz});
			}
		}
	}
	return re;
}

int hoge[256][256][256];

#ifdef NUIP
#define out(args...){vector<string> a_r_g_s=s_p_l_i_t(#args, ','); e_r_r(a_r_g_s.begin(), args); }
vector<string> s_p_l_i_t(const string &s, char c){vector<string> v;int d=0,f=0;string t;for(char c:s){if(!d&&c==',')v.push_back(t),t="";else t+=c;if(c=='\"'||c=='\'')f^=1;if(!f&&c=='(')++d;if(!f&&c==')')--d;}v.push_back(t);return move(v);}
void e_r_r(vector<string>::iterator it) {}
template<typename T, typename... Args> void e_r_r(vector<string>::iterator it, T a, Args... args){ if(*it==" 1"||*it=="1") cerr<<endl; else cerr << it -> substr((*it)[0] == ' ', it -> length()) << " = " << a << ", "; e_r_r(++it, args...);}
#else
#define out
#endif
#ifdef __cpp_init_captures
template<typename T>vector<T> table(int n, T v){ return vector<T>(n, v);}
template <class... Args> auto table(int n, Args... args){auto val = table(args...); return vector<decltype(val)>(n, move(val));}
#endif

map<pair<Vec3,Vec3>,vector<pair<Vec3,Vec3>>> Gspan(VoxelGrid mat){
	int dx[]={1,0,0,-1, 0, 0};
	int dy[]={0,1,0, 0,-1, 0};
	int dz[]={0,0,1, 0, 0,-1};
	map<pair<Vec3,Vec3>,vector<pair<Vec3,Vec3>>> re;
	queue<pair<Vec3, Vec3>> que;
	que.emplace(Vec3{0,-1,0}, Vec3{mat.r(), -1, mat.r()});
	for(int z=0; z<mat.r(); ++z)
		for(int y=0; y<mat.r(); ++y)
			for(int x=0; x<mat.r(); ++x) hoge[z][y][x]=mat(z,y,x);
	auto count=[](Vec3 lb, Vec3 rb)->int{
							 int re=0;
							 for(int x=lb.x; x<rb.x; ++x)
								 for(int y=lb.y; y<rb.y; ++y)
									 for(int z=lb.z; z<rb.z; ++z) re+=hoge[z][y][x];
							 return re;
						 };
	auto add=[](Vec3 pos, int val){
						 hoge[pos.z][pos.y][pos.x]+=val;
					 };
	auto push=[&](Vec3 lb,Vec3 rb, Vec3 fd){ // [lb,rb]からfd
							out(lb,rb,fd,1);
							int vol=(fd.x+1)*(fd.y+1)*(fd.z+1);
							for(int x=max(0, lb.x); x<=min(rb.x, mat.r()-fd.x); ++x){
								for(int y=max(0, lb.y); y<=min(rb.y, mat.r()-fd.y); ++y){
									for(int z=max(0, lb.z); z<=min(rb.z, mat.r()-fd.z); ++z){
										out(count(Vec3{x,y,z}, Vec3{x+1,y+1,z+1}+fd),1);
										if(count(Vec3{x,y,z}, Vec3{x+1,y+1,z+1}+fd) == vol){
											return Vec3{x,y,z};
										}
									}
								}
							}
							return Vec3{-1,-1,-1};
						};
	while(que.size()){
		Vec3 l,r;
		tie(l,r)=que.front(); que.pop();
		out(l,r,1);
#define ADJ\
		while(1){\
			int maxVol=0;\
			Vec3 a,b;\
			for(int dx=0; dx<30; ++dx){\
				bool foundX=false;\
				for(int dy=0; dy<30; ++dy){\
					bool foundY=false;\
					for(int dz=0; dz<30; ++dz){\
						Vec3 d{dx,dy,dz};\
						auto pos=push(BABABA, d);\
						if(pos.x<0) break;\
						foundY=foundX=true;\
						int vol=(dx+1)*(dy+1)*(dz+1);\
						if(maxVol < vol){\
							maxVol = vol;\
							a=pos; b=pos+d;\
						}\
					}\
					if(!foundY) break;\
				}\
				if(!foundX) break;\
			}\
			if(maxVol==0) break;\
			re[make_pair(l,r)].emplace_back(a,b);\
			que.emplace(a,b);\
			for(int x=min(a.x,b.x); x<=max(a.x,b.x); ++x){\
				for(int y=min(a.y,b.y); y<=max(a.y,b.y); ++y){\
					for(int z=min(a.z,b.z); z<=max(a.z,b.z); ++z){\
						add(Vec3{x,y,z}, -1);\
					}\
				}\
			}\
		}
#define BABABA Vec3{r.x+1, l.y+dy, l.z-dz}, Vec3{r.x+1, r.y, r.z}
		ADJ;
#undef BABABA
#define BABABA Vec3{l.x-dx, r.y+1, l.z-dz}, Vec3{r.x, r.y+1, r.z}
		ADJ;
#undef BABABA
#define BABABA Vec3{l.x-dx, l.y-dy, r.z+1}, Vec3{r.x, r.y, r.z+1}
		ADJ;
#undef BABABA
#define BABABA Vec3{l.x-1-dx, l.y-dy, l.z-dz}, Vec3{l.x-1-dx, r.y, r.z}
		ADJ
#undef BABABA
#define BABABA Vec3{l.x-dx, l.y-1-dy, l.z-dz}, Vec3{r.x, l.y-1-dy, r.z}
		ADJ
#undef BABABA
#define BABABA Vec3{l.x-dx, l.y-dy, l.z-1-dz}, Vec3{r.x, r.y, l.z-1-dz}
		ADJ
#undef BABABA
#undef ADJ
	}
	return re;
}

template<class S, class T> ostream& operator<<(ostream &os, const pair<S,T> &t) { return os<<"("<<t.first<<","<<t.second<<")";}


int main(int argc, char* argv[]){
  ios_base::sync_with_stdio(false);
  cout<<fixed<<setprecision(0);

	VoxelGrid v = read_data(argv[1]);

	{
		auto g=Gspan(v);
		for(auto p:g){
			for(auto v:p.second){
				cout<<"\""<<p.first<<"\" \""<<v<<"\""<<endl;
			}
		}
		return 0;
	}

	Tree poyo(v);

	for(auto v:poyo.getLeaves()){
		cout<<v<<",";
	}
	cout<<endl;

	poyo.eraseLeaf(Vec3{2,1,1});

	for(auto v:poyo.getLeaves()){
		cout<<v<<",";
	}
	cout<<endl;

	poyo.eraseLeaf(Vec3{1,1,1});

	for(auto v:poyo.getLeaves()){
		cout<<v<<",";
	}
	cout<<endl;

	//------------
	auto g=span(v);

	// ここの出力は
	// https://csacademy.com/app/graph_editor/
	// でビジュアライズできる
	// for(auto p:g){
	// 	for(auto v:p.second){
	// 		cout<<"\""<<p.first<<"\" \""<<v<<"\""<<endl;
	// 	}
	// }
  return 0;
}
