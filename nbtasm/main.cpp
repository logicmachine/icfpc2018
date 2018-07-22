#include "nbtasm.hpp"

void dump(const State& s){
	for(size_t i = 0; i < s.num_bots(); ++i){
		std::cout << i << ": " << s.bots(i) << std::endl;
	}
	std::cout << std::endl;
}

int main(){
	State s(20, 40);

	s.bots(0).fission(Vec3(1, 0, 0), 20);
	s.dump_pending_commands(std::cerr);
	s.commit();

	s.bots(0).smove(Vec3(0, 0, 1));
	s.bots(1).smove(Vec3(0, 0, 1));
	s.dump_pending_commands(std::cerr);
	s.commit();

	s.bots(0).wait();
	s.bots(1).smove(Vec3(15, 0, 0));
	s.dump_pending_commands(std::cerr);
	s.commit();

	s.bots(0).gfill(Vec3(1, 0, 0), Vec3(14, 0, 0));
	s.bots(1).gfill(Vec3(-1, 0, 0), Vec3(-14, 0, 0));
	s.dump_pending_commands(std::cerr);
	s.commit();

	s.bots(0).smove(Vec3(0, 0, -1));
	s.bots(1).smove(Vec3(0, 0, -1));
	s.dump_pending_commands(std::cerr);
	s.commit();

	s.bots(0).wait();
	s.bots(1).smove(Vec3(-15, 0, 0));
	s.dump_pending_commands(std::cerr);
	s.commit();

	s.bots(0).fusion_p(Vec3{ 1, 0, 0 });
	s.bots(1).fusion_s(Vec3{-1, 0, 0 });
	s.dump_pending_commands(std::cerr);
	s.commit();

	s.bots(0).halt();
	s.dump_pending_commands(std::cerr);
	s.commit();

	s.export_trace("trace.nbt");

	return 0;
}
