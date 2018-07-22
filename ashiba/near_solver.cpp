#include "bits/stdc++.h"
#define STRONG_VALIDATION
#include "../nbtasm/nbtasm.hpp"
#include "../nbtasm/backward.hpp"

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
//	map<Vec3,vector<Vec3>> originalTree;
//	map<Vec3,set<Vec3>> tree;
//	map<Vec3,Vec3> parent;
  map<Vec3, int> inDeg;
  map<Vec3, vector<Vec3>> rG;
  map<Vec3, int> dist;
	set<Vec3> leaves;
public:
	Tree(VoxelGrid mat){
		int dx[]={1,0,0,-1, 0, 0};
		int dy[]={0,1,0, 0,-1, 0};
		int dz[]={0,0,1, 0, 0,-1};
    dist[Vec3(-1, -1, -1)] = 0;

    queue<Vec3> que;
    for(int x=0; x<mat.r(); ++x){
			for(int z=0; z<mat.r(); ++z){
				if(mat(z,0,x)){
					que.push(Vec3(x, 0, z));
          dist[Vec3(x,0,z)] = 1;
				}
			}
    }

    assert(que.size());

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

        if( mat(nz, ny, nx) && dist.count(Vec3(nx, ny, nz))==0 ){
          assert(dist.count(pos));
          dist[Vec3(nx, ny, nz)] = dist[pos]+1;
					que.push(Vec3(nx, ny, nz));
				}
			}
		}

		for(auto &key:dist){
      Vec3 p = key.first;
      int maxi = -1;
      for(int d=0;d<6;d++){
        int ddz = p.z+dz[d];
        int ddy = p.y+dy[d];
        int ddx = p.x+dx[d];

        if(ddz<0 || ddy<0 || ddx<0 || ddz>=mat.r() || ddy>=mat.r() || ddx>=mat.r())continue;
        if( mat(ddz, ddy, ddx) ){
          assert(dist.count(Vec3(ddx, ddy, ddz)));
          if( dist[Vec3(p.x, p.y, p.z)]-dist[Vec3(ddx, ddy, ddz)]==1){
            rG[Vec3(p.x, p.y, p.z)].push_back(Vec3(ddx, ddy, ddz));
            inDeg[Vec3(ddx, ddy, ddz)]++;
          }
        }
  		}
  	}
    for(auto &key:rG){
      if(inDeg.count(key.first)==0)leaves.insert(key.first);
    }
  }

	const set<Vec3>& getLeaves(){ return leaves; }
	map<Vec3, int>& getDist(){ return dist; };

	// vector<Vec3> eraseLeaf(Vec3 leaf, set<Vec3> &cand){
	// 	assert(leaves.count(leaf));
  //   for(auto &elm:rG[leaf]){
  //     assert(inDeg>0);
  //     inDeg[elm]--;
  //     if(inDeg[elm]==0){
  //       cand.insert(elm);
  //       inDeg.elase(elm);
  //       leaves.insert(elm);
  //     }
  //   }
  //   leaves.elase(leaf);
	// }
  void eraseLeaf(Vec3 leaf, queue<Vec3> &newLeaves){
    assert(leaves.count(leaf));
    for(auto &elm:rG[leaf]){
      assert(inDeg[elm]>0);
      inDeg[elm]--;
      if(inDeg[elm]==0){
        inDeg.erase(elm);
        leaves.insert(elm);
				newLeaves.push(elm);
      }
    }
    leaves.erase(leaf);
  }

};

// vector<Vec3> isNextToLeaf(const &set<Vec3> leaf, Vec3, pos){
//   vector<Vec3> ret;
//   for(int dz=-1; dz<=1; dz++){
//     for(int dy=-1; dy<=1; dy++){
//       for(int dx=-1; dx<=1; dx++){
//         if(abs(dz)+abs(dy)+abs(dx)>=3 || max{abs(dz), abs(dy), abs(dx)})continue;
//         if(leaf.find(Vec3(dx, dy, dz))){
//           ret.push_back(Vec3(dx, dy, dz));
//         }
//       }
//     }
//   }
//   return ret;
// }
Vec3 isNextToLeaf(const set<Vec3> &leaf, Vec3 pos, int R){
  for(int dz=-1; dz<=1; dz++){
    for(int dy=-1; dy<=1; dy++){
      for(int dx=-1; dx<=1; dx++){
				if(dz==dy && dy==dx && dx==0)continue;
        if(abs(dz)+abs(dy)+abs(dx)>=3 || max(abs(dz), max(abs(dy), abs(dx)))>=2)continue;
        int ddz = dz+pos.z;
        int ddy = dy+pos.y;
        int ddx = dx+pos.x;
        if(ddz<0 || ddy<0 || ddx<0 || ddz>=R || ddy>=R || ddx>=R)continue;
        if(leaf.count(Vec3(ddx, ddy, ddz))) return Vec3(ddx, ddy, ddz) - pos;
      }
    }
  }
	return Vec3(-300, -300, -300);
}


void moveToLeaf(const set<Vec3> &leaf, State &s){
	static int dx[]={1,0,0,-1, 0, 0};
  static int dy[]={0,1,0, 0,-1, 0};
  static int dz[]={0,0,1, 0, 0,-1};
  Vec3 p = s.bots(0).pos();
  queue<Vec3> que; que.push(p);
  set<Vec3> vst; vst.insert(p);
  map<Vec3, Vec3> prv;
  prv[p] = Vec3{-1, -1, -1};

  while(que.size()){
    Vec3 p = que.front();
    que.pop();
    for(int d=0; d<6; d++){
      int ddz = p.z, ddy = p.y, ddx = p.x;
      for(int i=1;i<=15;i++){
        ddz+=dz[d], ddy+=dy[d], ddx+=dx[d];
        if(ddz<0 || ddy<0 || ddx<0 || ddz>=s.matrix().r() || ddy>=s.matrix().r() || ddx>=s.matrix().r())break;
        if((s.matrix())(ddz, ddy, ddx))break;
        Vec3 nxt(ddx, ddy, ddz);
        if(vst.emplace(nxt).second){
          assert(vst.count(nxt));
          prv[nxt] = p;
          que.push(nxt);
          Vec3 res = isNextToLeaf(leaf, nxt, s.matrix().r());
          if(res!=Vec3(-300, -300, -300)){
            vector<Vec3> root;
            for(Vec3 cur = nxt; cur.x>=0; cur=prv[cur]) root.push_back(cur);
            reverse(root.begin(), root.end());
            for(int i=0;i<root.size()-1;i++){
              s.bots(0).smove(root[i+1]-root[i]);
              s.commit();
            }
            return;
          }
        }

      }
    }
  }

  assert(0);
}


int main(int argc, char* argv[]){


	Vec3 dvec[]	=	{
		Vec3(1,0,0),
		Vec3(0,1,0),
		Vec3(0,0,1),
		Vec3(-1,0,0),
		Vec3(0,-1,0),
		Vec3(0,0,-1)
	};

  VoxelGrid vox = read_data(argv[1]);
  Tree graph(vox);
  assert(graph.getLeaves().size());

	// cout<<"There are these leaves."<<endl;
	// for(auto lf: graph.getLeaves()){
	// 	cout<<lf<<" "<<graph.getDist()[lf]<<endl;
	// }
	// cout<<string(20,'#')<<endl;

  State s(vox, 20);
  int R = vox.r();

  while(graph.getLeaves().size()){
		queue<Vec3> newLeaves;
    bool f = false;
    moveToLeaf(graph.getLeaves(), s);
    Vec3 pos = s.bots(0).pos();
    for(int dz=-1; dz<=1; dz++){
      for(int dy=-1; dy<=1; dy++){
        for(int dx=-1; dx<=1; dx++){
					if(dz==dy && dy==dx && dx==0)continue;
          if(abs(dz)+abs(dy)+abs(dx)>=3 || max(abs(dz), max(abs(dy), abs(dx)))>=2)continue;
          int ddz = dz+pos.z, ddy = dy+pos.y, ddx = dx+pos.x;
          if(ddz<0 || ddy<0 || ddx<0 || ddz>=R || ddy>=R || ddx>=R)continue;
          if(graph.getLeaves().count(Vec3(ddx, ddy, ddz))){
            s.bots(0).empty(Vec3(dx, dy, dz));
            s.commit();
						graph.eraseLeaf(Vec3(ddx, ddy, ddz), newLeaves);
            f=true;
          }
        }
      }
    }
    assert(f);
		while(newLeaves.size()){
			Vec3 tar = newLeaves.front();
			newLeaves.pop();
			if( graph.getLeaves().count(tar)==0 )continue;

			[&](){
				for(int d1 = 0; d1<6; d1++){
					Vec3 moved_pos = s.bots(0).pos();
					for(int d1_dist = 1; d1_dist<=5; d1_dist++){
						moved_pos += dvec[d1];
						if( not moved_pos.region_check(R) || s.matrix()(moved_pos.z, moved_pos.y, moved_pos.x))break;
						for(int d2 = 0; d2<6; d2++){
							if(d1==d2 || (d1+3)%6==d2)continue;
							Vec3 mpos = moved_pos;
							for(int d2_dist = 1; d2_dist<=5; d2_dist++){
								mpos += dvec[d2];
								if( not mpos.region_check(R) || s.matrix()(mpos.z, mpos.y, mpos.x))break;
								Vec3 res = isNextToLeaf(graph.getLeaves(), mpos, R);
								if( res != Vec3(-300, -300, -300) ){
									s.bots(0).lmove(dvec[d1]*d1_dist, dvec[d2]*d2_dist);
									s.commit();
									s.bots(0).empty(res);
									s.commit();
									graph.eraseLeaf(mpos+res, newLeaves);
									return;
								}
							}
						}
					}
				}
			}();
		}
  }


cout<<"last_part"<<endl;

  while(s.bots(0).pos().x){
		s.bots(0).smove(Vec3{-min(15,s.bots(0).pos().x),0,0});
		s.commit();
	}

	while(s.bots(0).pos().y){
		s.bots(0).smove(Vec3{0,-min(15,s.bots(0).pos().y),0});
		s.commit();
	}
	while(s.bots(0).pos().z){
		s.bots(0).smove(Vec3{0,0,-min(15,s.bots(0).pos().z)});
		s.commit();
	}


	s.bots(0).halt();
	s.commit();

	string file(argv[1]);
	{
		int n=file.size();
    for(int i=n-1;i>=0;i--) if(file[i]=='/'){
			file=file.substr(i+1);
			break;
		}
	}
	export_backward_trace((file.substr(0,5)+".nbt").c_str(), s);
  return 0;
}
