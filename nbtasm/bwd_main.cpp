#include "backward.hpp"

void dump(const State& s){
	for(size_t i = 0; i < s.num_bots(); ++i){
		std::cout << i << ": " << s.bots(i) << std::endl;
	}
	std::cout << std::endl;
}

int main(){
	VoxelGrid initial(10);
	initial(6, 0, 5) = 1;
	initial(6, 0, 6) = 1;

	State s(initial, 20);
	dump(s);

	s.bots(0).lmove(Vec3{ 0, 0, 5 }, Vec3{ 5, 0, 0 });
	s.commit();
	dump(s);

	s.bots(0).fission(Vec3{ -1, 0, 0 }, 5);
	s.commit();
	dump(s);

	s.bots(0).empty(Vec3{ 0, 0, 1 });
	s.bots(1).empty(Vec3{ 0, 0, 1 });
	s.commit();
	dump(s);
	for(int i = 0; i < 10; ++i){
		for(int j = 0; j < 10; ++j){ std::cout << (int)s.matrix()(i, 0, j); }
		std::cout << std::endl;
	}
	std::cout << std::endl;

	s.bots(0).smove(Vec3{ 0, 0, -5 });
	s.bots(1).smove(Vec3{ 0, 0, -5 });
	s.commit();
	dump(s);

	s.bots(0).fusion_s(Vec3{-1, 0, 0 });
	s.bots(1).fusion_p(Vec3{ 1, 0, 0 });
	s.commit();
	dump(s);

	s.bots(0).smove(Vec3{ -4, 0, 0 });
	s.commit();
	dump(s);

	s.bots(0).halt();
	s.commit();
	dump(s);

	export_backward_trace("trace.nbt", s);

	return 0;
}
