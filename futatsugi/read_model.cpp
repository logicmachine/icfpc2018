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
			v(cnt % R, (cnt / R) % R, cnt / (R * R)) = (x & 0x80) ? 1 : 0;
			x <<= 1;
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
			x <<= 1;
			if (cnt < R * R * R)
				x |= v(cnt % R, (cnt / R) % R, cnt / (R * R));
			cnt++;
		}
		fs.write((char*)&x, 1);
	}
	
	return 0;
}


int main(int argc, char* argv[])
{
	VoxelGrid v = read_data(argv[1]);
	int R = v.r();
	cout << "R: " << R << endl;

#if 1
	for (int x = 0; x < R; x++) {
		for (int y = 0; y < R; y++) {
			for (int z = 0; z < R; z++) {
				cout << static_cast<int>(v(z, y, x)) << ends;
			}
			cout << endl;
		}
		cout << endl;
	}
#endif

	write_data("temp.mdl", v);
	
	return 0;
}
