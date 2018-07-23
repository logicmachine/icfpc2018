#define STRONG_VALIDATION
#include "../../nbtasm/nbtasm.hpp"

#include "small.hpp"
#include "large.hpp"

int main(int argc, char *argv[]){
	const auto model = VoxelGrid::load_model(argv[1]);
	State s(model, 40);

	LargeSolver solve;
	solve(s);

	s.export_trace(argv[2]);

	return 0;
}
