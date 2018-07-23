#define STRONG_VALIDATION
#include "../../nbtasm/nbtasm.hpp"

#include "small.hpp"
#include "large.hpp"

int main(int argc, char *argv[]){
	const auto model = VoxelGrid::load_model(argv[1]);
	State s(model, 40);

	if(SmallSolver::is_solvable(s)){
		SmallSolver solve;
		solve(s);
	}else{
		LargeSolver solve;
		solve(s);
	}

	s.export_trace(argv[2]);
	std::cerr << "Energy: " << s.energy() << std::endl;

	return 0;
}
