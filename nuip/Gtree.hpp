#ifndef GTREE_HPP_
#define GTREE_HPP_
int fenwick[256][256][256];

class Gtree{
private:
	map<pair<Vec3,Vec3>,vector<pair<Vec3,Vec3>>> originalTree;
	map<pair<Vec3,Vec3>,set<pair<Vec3,Vec3>>> tree;
	map<pair<Vec3,Vec3>,pair<Vec3,Vec3>> parent;
	set<pair<Vec3,Vec3>> leaves,roots;

	int sum(Vec3 pos){
		pos+=Vec3{1,1,1};
		int s=0;
		for(int i=pos.z;i;i-=i&-i)
			for(int j=pos.y;j;j-=j&-j)
				for(int k=pos.x;k;k-=k&-k)
					s+=fenwick[i][j][k];
		return s;
	}
	int count(Vec3 lb, Vec3 rb){
		int s=0;
		s+=sum(rb);
		s-=sum(Vec3{lb.x,rb.y,rb.z});
		s-=sum(Vec3{rb.x,lb.y,rb.z});
		s-=sum(Vec3{rb.x,rb.y,lb.z});
		s+=sum(Vec3{lb.x,lb.y,rb.z});
		s+=sum(Vec3{rb.x,lb.y,lb.z});
		s+=sum(Vec3{lb.x,rb.y,lb.z});
		s-=sum(lb);
		return s;
	}
	void add(Vec3 pos, int val){
		pos+=Vec3{2,2,2};
		for(int i=pos.z;i<256;i+=i&-i)
			for(int j=pos.y;j<256;j+=j&-j)
				for(int k=pos.x;k<256;k+=k&-k)
					fenwick[i][j][k]+=val;
	}
	map<pair<Vec3,Vec3>,vector<pair<Vec3,Vec3>>> Gspan(VoxelGrid mat){
		int dx[]={1,0,0,-1, 0, 0};
		int dy[]={0,1,0, 0,-1, 0};
		int dz[]={0,0,1, 0, 0,-1};
		map<pair<Vec3,Vec3>,vector<pair<Vec3,Vec3>>> re;
		queue<pair<Vec3, Vec3>> que;
		que.emplace(Vec3{0,-1,0}, Vec3{mat.r()-1, -1, mat.r()-1});
		re[make_pair(Vec3{0,-1,0}, Vec3{mat.r()-1, -1, mat.r()-1})];
		auto push=[&](Vec3 lb,Vec3 rb, Vec3 fd){ // [lb,rb]‚©‚çfd
								int vol=(fd.x+1)*(fd.y+1)*(fd.z+1);
								for(int x=max(0, lb.x); x<=min(rb.x, mat.r()-1-fd.x); ++x){
									for(int y=max(0, lb.y); y<=min(rb.y, mat.r()-1-fd.y); ++y){
										for(int z=max(0, lb.z); z<=min(rb.z, mat.r()-1-fd.z); ++z){
											if(count(Vec3{x,y,z}, Vec3{x+1,y+1,z+1}+fd) == vol){
												return Vec3{x,y,z};
											}
										}
									}
								}
								return Vec3{-1,-1,-1};
							};
		fill(fenwick[0][0],fenwick[256][0],0);
		for(int z=0; z<mat.r(); ++z)
			for(int y=0; y<mat.r(); ++y)
				for(int x=0; x<mat.r(); ++x)if(mat(z,y,x)) add(Vec3{x,y,z},1);
		while(que.size()){
			Vec3 l,r;
			tie(l,r)=que.front(); que.pop();
#define ADJ																									\
			while(1){																							\
				int maxVol=0;																				\
				Vec3 a,b;																						\
				for(int dx=0; dx<30; ++dx){													\
					bool foundX=false;																\
					for(int dy=0; dy<30; ++dy){												\
						bool foundY=false;															\
						for(int dz=0; dz<30; ++dz){											\
							Vec3 d{dx,dy,dz};															\
							auto pos=push(BABABA, d);											\
							if(pos.x<0) break;														\
							foundY=foundX=true;														\
							int vol=(dx+1)*(dy+1)*(dz+1);									\
							if(maxVol < vol){															\
								maxVol = vol;																\
								a=pos; b=pos+d;															\
							}																							\
						}																								\
						if(!foundY) break;															\
					}																									\
					if(!foundX) break;																\
				}																										\
				if(maxVol==0) break;																\
				re[make_pair(l,r)].emplace_back(a,b);								\
				que.emplace(a,b);																		\
				re[make_pair(a,b)];																	\
				for(int x=min(a.x,b.x); x<=max(a.x,b.x); ++x){			\
					for(int y=min(a.y,b.y); y<=max(a.y,b.y); ++y){		\
						for(int z=min(a.z,b.z); z<=max(a.z,b.z); ++z){	\
							add(Vec3{x,y,z}, -1);													\
						}																								\
					}																									\
				}																										\
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
public:
	Gtree(VoxelGrid mat):roots{make_pair(Vec3{0, -1, 0}, Vec3{mat.r()-1, -1, mat.r()-1})}{
		originalTree=Gspan(mat);
		for(auto &p:originalTree){
			if(p.second.size()){
				tree[p.first]=set<pair<Vec3,Vec3>>(p.second.begin(), p.second.end());
			}else{
				leaves.emplace(p.first);
			}
			for(auto &cld:p.second) parent[cld]=p.first;
		}

		eraseRoot(*roots.begin());

		// for(auto p:originalTree){
		// 	for(auto v:p.second){
		// 		cout<<"\""<<p.first<<"\" \""<<v<<"\""<<endl;
		// 	}
		// }
	}

	const set<pair<Vec3,Vec3>>& getLeaves()const{ return leaves;}

	void eraseLeaf(pair<Vec3,Vec3> leaf){
		assert(leaves.count(leaf));
		auto par=parent[leaf];
		tree[par].erase(leaf);
		if(tree[par].empty()) leaves.emplace(par);
		leaves.erase(leaf);
	}

	const set<pair<Vec3,Vec3>>& getRoots()const{ return roots;}

	void eraseRoot(pair<Vec3,Vec3> root){
		assert(roots.count(root));
		for(auto cld:tree[root]){
			roots.insert(cld);
			//upd parent[cld]?
		}
		roots.erase(root);
	}
};

#endif
