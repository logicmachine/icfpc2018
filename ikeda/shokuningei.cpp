#include "nbtasm.hpp"
#include <bits/stdc++.h>

using namespace std;

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

void writeToConsole(VoxelGrid &vox)
{
    for (int i = 0; i < vox.r(); i++) {
        for (int j = 0; j < vox.r(); j++) {
            for (int k = 0; k < vox.r(); k++) {
                cout << (vox(j, i, k) ? 1 : 0);
            }
            cout << endl;
        }
        cout << "\n" << " === " << i << " === " << endl;
    }
}

void umbrella(void)
{
    VoxelGrid vox(10);

    for (int i = 0; i < 8; i++) {
        // z, y, x
        vox(8, i, 8) = 1;
    }

    for (int i = 1; i < 9; i++) {
        for (int j = 1; j < 9; j++) {
            // z, y, x
            vox(i, 8, j) = 1;
        }
    }
    //writeToConsole(vox);
    write_data("umbrella.mdl", vox);
}

void test(void)
{
    VoxelGrid vox(3);

    vox(1, 0, 1) = 1;

    write_data("test.mdl", vox);
}

void T(void)
{
    int r = 7;
    VoxelGrid vox(r);

    for (int i = 1; i < r-1; i++) {
        vox(r/2, r-2, i) = 1;
    }

    for (int i = 0; i < r-2; i++) {
        vox(r/2, i, r/2) = 1;
    }

    //writeToConsole(vox);
    //write_data("tube3x3.mdl", vox);
}

void tube(int size)
{
    int r = size;
    VoxelGrid vox(r);

    for (int i = 0; i < r-1; i++) {
        for (int j = r / 3; j < r - (r / 3); j++) {
            for (int k = r / 3; k < r - (r / 3); k++) {
                vox(j, i, k) = 1;
            }
        }
    }
    write_data("tube3x3.mdl", vox);
    //writeToConsole(vox);
}

void treeone()
{
    int r = 5;
    VoxelGrid v(r);

    v(2, 0, 2) = 1;

    v(1, 1, 2) = v(2, 1, 2) = v(3, 1, 2) = 1;

    v(1, 2, 1) = v(1, 2, 2) = v(1, 2, 3) = 1;
    v(3, 2, 1) = v(3, 2, 2) = v(3, 2, 3) = 1;

    v(1, 3, 1) = v(3, 3, 1) = v(1, 3, 3) = v(3, 3, 3) = 1;

    v(1, 4, 1) = v(3, 4, 1) = v(1, 4, 3) = v(3, 4, 3) = 1;

    write_data("tree.mdl", v);
}

int main(void)
{
    treeone();
    return 0;
}

