#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>


using namespace std;

// class VoxelGrid {

// private:
// 	int m_r;
// 	std::vector<uint8_t> m_grid;

// public:
// 	VoxelGrid()
// 		: m_grid()
// 	{ }

// 	explicit VoxelGrid(int r)
// 		: m_grid(r * r * r)
// 		, m_r(r)
// 	{ }

// 	uint8_t operator()(int i, int j, int k) const {
// 		const int r = m_r;
// 		return m_grid[i * r * r + j * r + k];
// 	}
// 	uint8_t& operator()(int i, int j, int k){
// 		const int r = m_r;
// 		return m_grid[i * r * r + j * r + k];
// 	}

// 	int r() const {
// 		return m_r;
// 	}

// };

#include "nbtasm.hpp"

void _dump(const State& s){
	for(size_t i = 0; i < s.num_bots(); ++i){
		std::cout << i << ": " << s.bots(i) << std::endl;
	}
	std::cout << std::endl;
}

#if 0
#define dump(s) _dump(s)
#define dout(s) cout << #s << s << endl
#else
#define dump(s)
#define dout(s)
#endif

//using Int = long;

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

struct UnionFind;

State s;
VoxelGrid model;
int R;
Vec3 modelMin, modelMax;
int searchXDir;
int searchZDir;
int connectedCompNum;
bool harmonicsHigh;

//*************
int Vec2Int(Vec3 v) {
	return v.x + v.y*R + v.z*R*R;
}

void clampInt(int& x, int min, int max){
	if (x < min) x = min;
	if (max < x) x = max;	
}
void clampVec3(Vec3& v, const Vec3& min, const Vec3& max) {
	clampInt(v.x, min.x, max.x);
	clampInt(v.y, min.y, max.y);
	clampInt(v.z, min.z, max.z);
}

struct UnionFind {
  vector<int> data;
  UnionFind() : UnionFind(0){}
  UnionFind(int size) : data(size, -1){ }

  bool unionSet(int x, int y) {
    x = root(x); y = root(y);
    if (x != y) {
      if (data[y] < data[x]) swap(x, y);
      data[x] += data[y]; data[y] = x;
    }
    return x != y;
  }
  bool unionSet(Vec3 x, Vec3 y) {
	  return unionSet(Vec2Int(x), Vec2Int(y));
  }
  bool findSet(int x, int y) {
    return root(x) == root(y);
  }
  bool findSet(Vec3 x, Vec3 y) {
	  return findSet(Vec2Int(x), Vec2Int(y));
  }
  int root(int x) {
    return data[x] < 0 ? x : data[x] = root(data[x]);
  }
  int root(Vec3 x){return root(Vec2Int(x));}
  int size(int x) {
    return -data[root(x)];
  }
  int size(Vec3 x) {return size(Vec2Int(x));}
};
UnionFind connectedUF;

//*************
void test_unionFind(){
	R = 3;
	UnionFind uf(R*R*R);
	cout << uf.root(Vec3{0,0,0}) << endl;
	cout << uf.root(Vec3{1,1,1}) << endl;
	cout << uf.root(Vec3{2,2,2}) << endl;
	cout << uf.size(Vec3{1,1,1}) << endl;
	uf.unionSet(Vec3{1,1,1}, Vec3{2,2,2});
	cout << uf.root(Vec3{0,0,0}) << endl;
	cout << uf.root(Vec3{1,1,1}) << endl;
	cout << uf.root(Vec3{2,2,2}) << endl;
	cout << uf.size(Vec3{1,1,1}) << endl;
}

//*************

Vec3 getTargetBox(Vec3 searchSrc) {
	while (searchSrc != (modelMax + Vec3{0,1,0})){
		if (model(searchSrc.z, searchSrc.y, searchSrc.x))
			return searchSrc;
		// next box
		searchSrc.x += searchXDir;
		if (searchSrc.x < modelMin.x || modelMax.x < searchSrc.x) {
			searchXDir *= -1;
			searchSrc.x = (searchXDir == 1) ? modelMin.x : modelMax.x;
			searchSrc.z += searchZDir;
		}
		if (searchSrc.z < modelMin.z || modelMax.z < searchSrc.z) {
			searchZDir *= -1;
			searchSrc.z = (searchZDir == 1) ? modelMin.z : modelMax.z;
			searchSrc.y += 1;
		}
		//cout << "next box" << searchSrc << endl;
	}
	return searchSrc;
}

void moveBot(State::BotReference bot, Vec3 target){
	Vec3 d = target - bot.pos();
	clampVec3(d, Vec3{-15, -15, -15}, Vec3{15,15,15});
	if (d.x != 0){
		d.y = 0; d.z = 0;
		bot.smove(d);
	} else if (d.z != 0) {
		d.y = 0;
		bot.smove(d);
	} else if (d.y != 0){
		bot.smove(d);
	} else {
		bot.wait();
	}
}

// 次に置く場所fillBox　で連結成分どうなるかを計算．
void connectCheck(Vec3 fillBox) {
	vector<Vec3> connectPoss;

	for (Vec3 d : {Vec3{1,0,0}, Vec3{-1,0,0}, Vec3{0,1,0}, Vec3{0,-1,0}, Vec3{0,0,1}, Vec3{0,0,-1} }) {
		Vec3 nb = fillBox + d;
		if (nb.x < 0 || nb.y < 0 || nb.z < 0 ||
			R <= nb.x || R <= nb.y || R <= nb.z) {
			continue;
		}
		if (s.matrix()(nb.z, nb.y, nb.x) != 1){
			continue;
		}
		connectPoss.push_back(nb);
	}
	if (connectPoss.size() == 0) {
		connectedCompNum++;
		return;
	} else {
		connectedUF.unionSet(fillBox, connectPoss[0]);
		for (int i = 1; i < connectPoss.size(); i++) {
			 if (connectedUF.unionSet(fillBox, connectPoss[i])) {
				 connectedCompNum--;
			 }
		}
	}
}

void solve(char** argv){
	model = read_data(argv[1]);
	R = model.r();
	cout << "R: " << R << endl;
	s = State(R, 20);
	connectedCompNum = 0;
	connectedUF = UnionFind(R*R*R);

#if 0
	for (int y = 0; y < R; y++) {
		for (int z = 0; z < R; z++) {
			for (int x = 0; x < R; x++) {
				cout << static_cast<int>(model(z, y, x)) << ends;
			}
			cout << endl;
		}
		cout << endl;
	}
	return;
#endif

	modelMin = Vec3{R-1,R-1,R-1};
	modelMax = Vec3{0,0,0};
	searchXDir = 1;
	searchZDir = 1;

	for (int x = 0; x < R; x++) {
		for (int y = 0; y < R; y++) {
			for (int z = 0; z < R; z++) {
				Vec3 v = Vec3{x,y,z};
				if (model(z,y,x)){
					modelMin.x = (x < modelMin.x) ? x : modelMin.x;
					modelMin.y = (y < modelMin.y) ? y : modelMin.y;
					modelMin.z = (z < modelMin.z) ? z : modelMin.z;
					modelMax.x = (modelMax.x < x) ? x : modelMax.x;
					modelMax.y = (modelMax.y < y) ? y : modelMax.y;
					modelMax.z = (modelMax.z < z) ? z : modelMax.z;
				}
			}
		}
	}
	cout << "modelMin" << modelMin << endl;
	cout << "modelMax" << modelMax << endl;
	
	// harmonic専任bot生成
	s.bots(0).fission(Vec3{0,0,1}, 0);
	s.commit();
	
	s.bots(0).wait();
	s.commit();

	Vec3 targetBox = modelMin;
	while (true) {
		// get next  targetBox
		targetBox = getTargetBox(targetBox);

		if (model(targetBox.z, targetBox.y, targetBox.x) != 1){
			break;
		}
		// targetBoxの真上へ move
		Vec3 targetBoxU = targetBox + Vec3{0,1,0};
		dout(targetBoxU);
		while (s.bots(0).pos() != targetBoxU) {
			moveBot(s.bots(0), targetBoxU);
			s.bots(1).wait();
			s.commit();
			dump(s);
		}
		// fill
		connectCheck(targetBox);
		if ((connectedCompNum > 1 && harmonicsHigh == false) ||
		    (connectedCompNum <= 1 && harmonicsHigh == true)) {
			harmonicsHigh = !harmonicsHigh;
			//cout << "flip" << endl;
			
			s.bots(1).flip();
		} else {
			s.bots(1).wait();
		}
		s.bots(0).fill(Vec3{0,-1,0});
		model(targetBox.z, targetBox.y, targetBox.x) = 0;
		s.commit();
		dump(s);
		dout(connectedCompNum);

	}
	// 帰る
	Vec3 targetPos;
	targetPos.y = s.bots(0).pos().y;
	while (s.bots(0).pos() != targetPos) {
		moveBot(s.bots(0), targetPos);
		s.bots(1).wait();
		s.commit();
		dump(s);
	}
	targetPos = Vec3{0,0,0};
	while (s.bots(0).pos() != targetPos) {
		moveBot(s.bots(0), targetPos);
		s.bots(1).wait();
		s.commit();
		dump(s);
	}
	
	s.bots(0).fusion_p(Vec3{0,0, 1});
	s.bots(1).fusion_s(Vec3{0,0,-1});
	s.commit();
	dump(s);
	
	s.bots(0).halt();
	s.commit();
	dump(s);

	s.export_trace("trace.nbt");
}
int main(int argc, char* argv[])
{
	//test_unionFind();
	solve(argv);


	//write_data("temp.mdl", model);
	
	return 0;
}
