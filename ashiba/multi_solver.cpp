#include "bits/stdc++.h"
#define STRONG_VALIDATION
const int MAX_LONG_DISTANCE = 15;
#include "../nbtasm/nbtasm.hpp"
#include "../nbtasm/backward.hpp"
#include "../logicmachine/libnbt/collect_nanobots.hpp"
#include <ctime>
#include <deque>
const int SEARCH_TIMES = 100;

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
		if(tree[par].empty() && par!=Vec3(-1, -1, -1)) leaves.emplace(par);
		leaves.erase(leaf);
	}
};



struct PosData{
	Vec3 pos;
	int times;
};

int mlen(Vec3 a, Vec3 b){
	return abs(a.x-b.x) + abs(a.y-b.y) + abs(a.z-b.z);
}


void getPath(const Vec3 &pos, map<Vec3, Vec3> &prv, vector<Vec3> &path){
	Vec3 cur = pos;
	while(prv[cur]!=Vec3(-1, -1, -1)){
		// cout<<"cur: "<<cur<<endl;
		path.push_back( cur - prv[cur] );
		cur = prv[cur];
	}
	reverse(path.begin(), path.end());
}




bool isReachableLeafND(const Vec3 &pos, const set<Vec3> &leaves, const int &R, const set<Vec3> &reserved){
	for(int dz=-1; dz<=1; dz++){
    for(int dy=-1; dy<=1; dy++){
      for(int dx=-1; dx<=1; dx++){
        if(abs(dz)+abs(dy)+abs(dx)>=3 || max(abs(dz), max(abs(dy), abs(dx)))>=2)continue;
        int ddz = dz+pos.z;
        int ddy = dy+pos.y;
        int ddx = dx+pos.x;
        if(ddz<0 || ddy<0 || ddx<0 || ddz>=R || ddy>=R || ddx>=R)continue;
        if(leaves.count(Vec3(ddx, ddy, ddz)) && reserved.count(Vec3(ddx, ddy, ddz))==0) return true;
      }
    }
  }
  return false;
}

Vec3 getReachableLeavesPoint(const Vec3 &pos, const set<Vec3> &leaves, const int &R, const set<Vec3> &reserved){
	for(int dz=-1; dz<=1; dz++){
    for(int dy=-1; dy<=1; dy++){
      for(int dx=-1; dx<=1; dx++){
        if(abs(dz)+abs(dy)+abs(dx)>=3 || max(abs(dz), max(abs(dy), abs(dx)))>=2)continue;
        int ddz = dz+pos.z;
        int ddy = dy+pos.y;
        int ddx = dx+pos.x;
        if(ddz<0 || ddy<0 || ddx<0 || ddz>=R || ddy>=R || ddx>=R)continue;
        if(leaves.count(Vec3(ddx, ddy, ddz)) && reserved.count(Vec3(ddx, ddy, ddz))==0) return Vec3(ddx, ddy, ddz);
      }
    }
  }
  return Vec3(-300, -300, -300);
}

string getMovingPath(const State &s, const int &botn, const set<Vec3> &leaves, vector<Vec3> &path, const VoxelGrid &vCood, set<Vec3> &reserved, const bool & emer){
	int R = s.matrix().r();
	int dx[]={1,0,0,-1,0,0};
	int dy[]={0,0,1,0,0,-1};
	int dz[]={0,1,0,0,-1,0};

	Vec3 pos = s.bots(botn).pos();
  queue<PosData> que; que.push(PosData{pos, 0});
  set<Vec3> vst; vst.insert(pos);
  map<Vec3, Vec3> prv;
  prv[pos] = Vec3{-1, -1, -1};

	bool does_exist_valid_moving = false;

  while(que.size()){
		PosData tmp = que.front();
		Vec3 p = tmp.pos;
		que.pop();

		if( not emer && tmp.times+1<SEARCH_TIMES ){
			break;
		}

		for(int d=0; d<6; d++){
			int ddz = p.z, ddy = p.y, ddx = p.x;
			for(int dist=1; dist<=15; dist++){
				ddz+=dz[d], ddy+=dy[d], ddx+=dx[d];

				Vec3 nxt(ddx, ddy, ddz);
				if( not nxt.region_check(R) || s.matrix()(ddz, ddy, ddx) || vCood(ddz, ddy, ddx))break;

				if(vst.insert(nxt).second){
					// cout<<"add "<<nxt<<" as visited"<<endl;
					does_exist_valid_moving = true;
					prv[nxt] = p;

					if( isReachableLeafND(nxt, leaves, R, reserved) ){
						// cout<<"found path!"<<endl;
						getPath(nxt, prv, path);
						// cout<<"got path"<<endl;
						Vec3 lf_pos = getReachableLeavesPoint(nxt, leaves, R, reserved);
						assert( leaves.count(lf_pos) );
						assert( reserved.insert(lf_pos).second );
						return "MOVE_TO_LEAF";
					}
					que.push(PosData{nxt, tmp.times+1});
				}
			}
		}
	}
	if( not does_exist_valid_moving ){
		return "NO_ROOT";
	}
	else{
		Vec3 dvec[]	=	{
			Vec3(1,0,0),
			Vec3(0,1,0),
			Vec3(0,0,1),
			Vec3(-1,0,0),
			Vec3(0,-1,0),
			Vec3(0,0,-1)
		};

		int d1 = rand()%6;
		int d1_dist = rand()%5+11;

		Vec3 cur = s.bots(botn).pos();
		for(int i=1; i<=d1_dist; i++){
			cur+= dvec[d1];
			if( not cur.region_check(R) || s.matrix()(cur.z, cur.y, cur.x) || vCood(cur.z, cur.y, cur.x)){
				cur -= dvec[d1];
				break;
			}
			// cout<<"along path"<<cur<<endl;
		}

		if( cur==s.bots(botn).pos() ){
			assert(path.size()==0);
			return "RANDOM_FAILED";
		}else{
			prv[cur] = s.bots(botn).pos();
			// cout<<cur<<" to "<<s.bots(botn).pos()<<endl;
			getPath(cur, prv, path);

			return "RANDOM";
		}
	}
}


void fillVCoord(const Vec3 &s, const Vec3 &g, VoxelGrid &vCoord){
	// cout<<"fill"<<s<<" "<<g<<endl;
	Vec3 dvec[]	=	{
		Vec3(1,0,0),
		Vec3(0,1,0),
		Vec3(0,0,1),
		Vec3(-1,0,0),
		Vec3(0,-1,0),
		Vec3(0,0,-1)
	};

	Vec3 tmp = g-s;

	int ratio = abs(tmp.x?tmp.x: tmp.y?tmp.y:tmp.z);
	tmp.x/=ratio, tmp.y/=ratio, tmp.z/=ratio;

	int dir = -1;
	for(int d=0; d<6; d++){
		if(dvec[d]==tmp)dir = d;
	}

	assert(dir!=-1);

	tmp = s+dvec[dir];
	while(1){
		// assert( not vCoord(tmp.z, tmp.y, tmp.x) );
//		cout<<tmp.z<<" "<<tmp.y<<" "<<tmp.x<<endl;
		vCoord(tmp.z, tmp.y, tmp.x) = 1;
		// cout<<"peace"<<endl;
		if(tmp==g)break;
		else tmp += dvec[dir];

	}
}


int main(int argc, char* argv[]){
	srand((unsigned int)(time(NULL)));

	VoxelGrid vox = read_data(argv[1]);
	int R = vox.r();
	// SEARCH_TIMES = max(SEARCH_TIMES, R/30);
	int N = min(40, max((R-1)/3*3+1,4));
	// cout<<"R: "<<R<<endl;
	// cout<<"N: "<<N<<endl;

	State s(vox, N);

	//Fission to 3 direction
	for(int i=0; i<N; i++){
		if(i!=N-1) s.bots(0).fission(Vec3(i%3==0, i%3==1, i%3==2), 0);
		for(int j=1; j<s.num_bots(); j++){
			int id = (j-1+3)%3;
			if(i%3==id)continue;

			if(id==0){
				if(s.bots(j).pos().x<R-1){
					s.bots(j).smove( Vec3(min(1, R-s.bots(j).pos().x-1), 0, 0) );
				}
			}else if(id==1){
				if(s.bots(j).pos().y<R-1){
					s.bots(j).smove( Vec3(0, min(1, R-s.bots(j).pos().y-1), 0) );
				}
			}else{
				assert(id==2);
				if(s.bots(j).pos().z<R-1){
					s.bots(j).smove( Vec3(0, 0, min(1, R-s.bots(j).pos().z-1)) );
				}
			}
		}
		s.commit();

	}


	Tree graph(vox);
	assert( graph.getLeaves().size()>0 );

	int turn = 1;
	bool emer = false;
	while(graph.getLeaves().size()){
		vector<string> results(s.num_bots());
		// cout<<"Turn: "<<turn++<<", Remaining leaves: "<<graph.getLeaves().size()<<endl;
		// if(score.size()>=20)score.pop_front();
		// score.push_back(graph.getLeaves().size());
		// if(score.size()==20 && score.front()==score.back())break;
		// if(turn>2)break;
		VoxelGrid vCoord(R);

		//set each bot's position as volatil coordinate
		for(int botn=0; botn<s.num_bots(); botn++){
			vCoord(s.bots(botn).pos().z, s.bots(botn).pos().y, s.bots(botn).pos().x) = 1;
		}

		int num_queried_nanobots = 0;
		set<Vec3> reserved_leaves;
		for(int botn=0; botn<s.num_bots(); botn++){
			// cout<<"bot "<<botn<<" start"<<endl;
			if(reserved_leaves.size()==graph.getLeaves().size())continue;
			if( isReachableLeafND(s.bots(botn).pos(), graph.getLeaves(), R, reserved_leaves)){
				Vec3 v= getReachableLeavesPoint(s.bots(botn).pos(), graph.getLeaves(), R, reserved_leaves);
				assert( reserved_leaves.insert(v).second );

			}else{
				vector<Vec3> path;
				string result = getMovingPath(s, botn, graph.getLeaves(), path, vCoord, reserved_leaves, emer);
				results[botn] = result;


				if(path.size()){
					// cout<<"bot"<<botn<<" found a root"<<endl;
					assert( result=="MOVE_TO_LEAF" || "RANDOM" );
					// cout<<"do smove"<<endl;
					// cout<<result<<endl;
					s.bots(botn).smove(path[0]);
					// cout<<"done smove"<<endl;
					fillVCoord(s.bots(botn).pos(), s.bots(botn).pos()+path[0], vCoord);
					if(result=="MOVE_TO_LEAF"){
						num_queried_nanobots++;
					}
				}
			}
		}
		if( num_queried_nanobots == 0 ){
			for(int botn = 0; botn<s.num_bots(); botn++){
				Vec3 tar = getReachableLeavesPoint(s.bots(botn).pos(), graph.getLeaves(), R, set<Vec3>());
				if(tar==Vec3(-300, -300, -300))continue;
				s.bots(botn).empty(tar - s.bots(botn).pos());
				graph.eraseLeaf(tar);
			}
		}
		// s.dump_pending_commands(std::cerr);
		// for(int i=1; i<=s.num_bots(); i++){
		// 	cout<<"bot"<<i<<"result is"<<results[i-1]<<endl;
		// }
		s.commit();
		if(emer)emer = false;
		if(num_queried_nanobots<=N*1/3 )emer = true;
	}

	collect_nanobots_x(s);
	collect_nanobots_y(s);
	collect_nanobots_z(s);

	//Output results
	string file(argv[1]);
	{
		int n=file.size();
    for(int i=n-1;i>=0;i--) if(file[i]=='/'){
			file=file.substr(i+1);
			break;
		}
	}

	// cout<<"End of erasing."<<endl;
	s.bots(0).halt();
	s.commit();

	file=file.substr(0,5);
	file[1]='A';
	export_backward_trace((file.substr(0,5)+".nbt").c_str(), s);
	file[1]='D';
	s.export_trace((file.substr(0,5)+".nbt").c_str());


	return 0;
}
