#pragma once

#include "nbtasm.hpp"

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
