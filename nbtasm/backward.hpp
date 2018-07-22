#ifndef NBTASM_BACKWARD_H
#define NBTASM_BACKWARD_H

#include "nbtasm.hpp"

static void export_backward_trace(const std::string& filename, const State& bwd){
	State replay(bwd.matrix(), bwd.max_num_bots());
	const auto& bwd_trace = bwd.trace();
	std::vector<std::unordered_map<Vec3, Command>> pos2cmd;
	for(size_t i = 0; i < bwd_trace.size(); ){
		pos2cmd.emplace_back();
		for(size_t j = 0; j < replay.num_bots(); ++j, ++i){
			const auto& b = replay.bots(j);
			const auto& c = bwd_trace[i];
			if(c.type == CommandType::SMove){
				pos2cmd.back().emplace(
					b.pos() + c.smove_lld(),
					Command(CommandType::SMove).smove_lld(-c.smove_lld()));
			}else if(c.type == CommandType::LMove){
				pos2cmd.back().emplace(
					b.pos() + c.lmove_sld1() + c.lmove_sld2(),
					Command(CommandType::LMove)
						.lmove_sld1(-c.lmove_sld2())
						.lmove_sld2(-c.lmove_sld1()));
			}else if(c.type == CommandType::Fission){
				pos2cmd.back().emplace(b.pos(), Command(CommandType::FusionP).fusion_nd(c.fission_nd()));
				pos2cmd.back().emplace(
					b.pos() + c.fission_nd(),
					Command(CommandType::FusionS).fusion_nd(-c.fission_nd()));
			}else if(c.type == CommandType::Fill){
				pos2cmd.back().emplace(b.pos(), Command(CommandType::Empty).empty_nd(c.fill_nd()));
			}else if(c.type == CommandType::Empty){
				pos2cmd.back().emplace(b.pos(), Command(CommandType::Fill).fill_nd(c.empty_nd()));
			}else if(c.type == CommandType::FusionP){
				int secondary_seeds_size = 0;
				for(size_t k = 0; k < replay.num_bots(); ++k){
					if(replay.bots(k).pos() == b.pos() + c.fusion_nd()){
						secondary_seeds_size = __builtin_popcount(replay.bots(k).seeds());
					}
				}
				pos2cmd.back().emplace(
					b.pos(),
					Command(CommandType::Fission)
						.fission_nd(c.fusion_nd())
						.fission_m(secondary_seeds_size));
			}else if(c.type == CommandType::FusionS){
				// nothing to do
			}else if(c.type == CommandType::GFill){
				pos2cmd.back().emplace(
					b.pos(),
					Command(CommandType::GEmpty)
						.gempty_nd(c.gfill_nd())
						.gempty_fd(c.gfill_fd()));
			}else if(c.type == CommandType::GEmpty){
				pos2cmd.back().emplace(
					b.pos(),
					Command(CommandType::GFill)
						.gfill_nd(c.gempty_nd())
						.gfill_fd(c.gempty_fd()));
			}else{
				pos2cmd.back().emplace(b.pos(), c);
			}
			replay.bots(j).exec(bwd_trace[i]);
		}
		replay.commit();
	}

	State fwd(bwd.matrix().size(), bwd.max_num_bots());
	for(int i = pos2cmd.size() - 2; i >= 0; --i){
		for(size_t j = 0; j < fwd.num_bots(); ++j){
			const auto& b = fwd.bots(j);
			const auto it = pos2cmd[i].find(b.pos());
			fwd.bots(j).exec(it->second);
		}
		fwd.commit();
	}
	fwd.bots(0).halt();
	fwd.commit();
	fwd.export_trace(filename);
}

#endif
