#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>

using namespace std;

class VoxelGrid {

private:
	int m_r;
	std::vector<uint8_t> m_grid;

public:
	VoxelGrid()
		: m_grid()
	{ }

	explicit VoxelGrid(int r)
		: m_grid(r * r * r)
		, m_r(r)
	{ }

	uint8_t operator()(int i, int j, int k) const {
		const int r = m_r;
		return m_grid[i * r * r + j * r + k];
	}
	uint8_t& operator()(int i, int j, int k){
		const int r = m_r;
		return m_grid[i * r * r + j * r + k];
	}

	int r() const {
		return m_r;
	}

};

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



int main(int argc, char* argv[])
{

	{
		VoxelGrid v(4);
		v(1,1,1)=v(1,1,2)=1;
		v(2,0,1)=v(2,1,1)=v(2,2,1)=v(2,2,2)=1;
		write_data("F.mdl", v);
	}
	{
		VoxelGrid v(5);
		for(int z=0;z<3;++z)
			for(int y=0;y<3;++y)
				for(int x=0;x<3;++x) v(z+1,y,x+1)=1;
		v(2,1,2)=0;
		write_data("Cube3x3Hollow.mdl", v);
	}
	return 0;
}
