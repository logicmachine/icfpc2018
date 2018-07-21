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


int main(int argc, char* argv[]){
  ios_base::sync_with_stdio(false);
  cout<<fixed<<setprecision(0);

	VoxelGrid v = read_data("testcase/F.mdl");

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
