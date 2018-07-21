#ifndef NBTASM_BACKWARD_H
#define NBTASM_BACKWARD_H

#include "nbtasm.hpp"


class BackwardState {

public:
	class ConstBotReference {

	private:
		const detail::Bot& m_bot;

	public:
		explicit ConstBotReference(const detail::Bot& b)
			: m_bot(b)
		{ }

		int bid() const { return m_bot.bid; }
		const Vec3& pos() const { return m_bot.pos; }
		uint32_t seeds() const { return m_bot.seeds; }

	};


	class BotReference {

	private:
		BackwardState& m_state;
		detail::Bot& m_bot;

		detail::Command& m_command;

	public:
		BotReference(BackwardState& s, detail::Bot& b, detail::Command& c)
			: m_state(s)
			, m_bot(b)
			, m_command(c)
		{ }

		int bid() const { return m_bot.bid; }
		const Vec3& pos() const { return m_bot.pos; }
		uint32_t seeds() const { return m_bot.seeds; }

		void halt(){ m_command.type = detail::CommandType::Halt; }
		void wait(){ m_command.type = detail::CommandType::Wait; }
		void flip(){ m_command.type = detail::CommandType::Flip; }
		void smove(const Vec3& lld){
			detail::throw_test_long_distance(lld);
			detail::throw_test_position_range(m_state.matrix().r(), m_bot.pos + lld);
			m_command.type = detail::CommandType::SMove;
			m_command.u.smove = detail::SMoveParams{ lld };
		}
		void lmove(const Vec3& sld1, const Vec3& sld2){
			detail::throw_test_short_distance(sld1);
			detail::throw_test_short_distance(sld2);
			detail::throw_test_position_range(m_state.matrix().r(), m_bot.pos + sld1 + sld2);
			m_command.type = detail::CommandType::LMove;
			m_command.u.lmove = detail::LMoveParams{ sld1, sld2 };
		}
		void fission(const Vec3& nd, int m){
			detail::throw_test_near_distance(nd);
			const int n = __builtin_popcount(m_bot.seeds);
			if(m < 0 || n < m + 1){ throw std::runtime_error("fission: invalid m"); }
			m_command.type = detail::CommandType::Fission;
			m_command.u.fission = detail::FissionParams{ nd, m };
		}
		void empty(const Vec3& nd){
			detail::throw_test_near_distance(nd);
			m_command.type = detail::CommandType::Fill;
			m_command.u.fill = detail::FillParams{ nd };
		}
		void fusion_p(const Vec3& nd){
			detail::throw_test_near_distance(nd);
			m_command.type = detail::CommandType::FusionP;
			m_command.u.fusion = detail::FusionParams{ nd };
		}
		void fusion_s(const Vec3& nd){
			detail::throw_test_near_distance(nd);
			m_command.type = detail::CommandType::FusionS;
			m_command.u.fusion = detail::FusionParams{ nd };
		}

		void exec(const detail::Command& cmd){
			m_command = cmd;
		}

	};


private:
	int64_t m_energy;
	Harmonics m_harmonics;
	VoxelGrid m_matrix;
	std::vector<detail::Bot> m_bots;

	std::vector<detail::Command> m_pending_commands;

	std::vector<std::map<Vec3, detail::Command>> m_pos2commands;
	int m_initial_bid;
	uint32_t m_initial_seeds;

	detail::Command make_backward_command(const detail::Command& c) const {
		detail::Command out = c;
		if(c.type == detail::CommandType::SMove){
			out.u.smove.lld = -c.u.smove.lld;
		}else if(c.type == detail::CommandType::LMove){
			out.u.lmove.sld1 = -c.u.lmove.sld1;
			out.u.lmove.sld2 = -c.u.lmove.sld2;
		}else if(c.type == detail::CommandType::Fission){
			out.type = detail::CommandType::FusionP;
			out.u.fusion.nd = c.u.fission.nd;
		}else if(c.type == detail::CommandType::FusionP){
			out.type = detail::CommandType::Fission;
			out.u.fission.nd = c.u.fusion.nd;
		}
		return out;
	}

public:
	BackwardState()
		: m_energy(0)
		, m_harmonics(Harmonics::Low)
		, m_matrix()
		, m_bots()
		, m_pending_commands()
		, m_pos2commands()
		, m_initial_bid(0)
		, m_initial_seeds(0)
	{ }

	explicit BackwardState(
		const VoxelGrid& target, int bid = 1, uint32_t seeds = 0x1ffffcu)
		: m_energy(0)
		, m_harmonics(Harmonics::Low)
		, m_matrix(target)
		, m_bots(1, detail::Bot{ bid, Vec3{ 0, 0, 0 }, seeds })
		, m_pending_commands(1)
		, m_pos2commands()
		, m_initial_bid(bid)
		, m_initial_seeds(seeds)
	{ }

	int energy() const { return m_energy; }
	Harmonics harmonics() const { return m_harmonics; }
	const VoxelGrid& matrix() const { return m_matrix; }

	ConstBotReference bots(size_t i) const { return ConstBotReference(m_bots[i]); }
	BotReference bots(size_t i){ return BotReference(*this, m_bots[i], m_pending_commands[i]); }
	size_t num_bots() const { return m_bots.size(); }

	void commit(){
		std::map<Vec3, size_t> pos2idx;
		for(size_t i = 0; i < m_bots.size(); ++i){ pos2idx[m_bots[i].pos] = i; }

		std::vector<detail::Bot> new_bots;
		std::map<Vec3, detail::Command> pos2cmd;
		for(size_t i = 0; i < m_bots.size(); ++i){
			const auto& b = m_bots[i];
			const auto& c = m_pending_commands[i];
			if(c.type == detail::CommandType::Empty){
				throw std::runtime_error("no commands are supplied");
			}else if(c.type == detail::CommandType::Halt){
				if(m_bots.size() != 1){ throw std::runtime_error("constraints is not satisfied for halt"); }
				new_bots.push_back(b);
				pos2cmd.emplace(new_bots.back().pos, make_backward_command(c));
			}else if(c.type == detail::CommandType::Wait){
				new_bots.push_back(b);
				pos2cmd.emplace(new_bots.back().pos, make_backward_command(c));
			}else if(c.type == detail::CommandType::Flip){
				if(m_harmonics == Harmonics::Low){
					m_harmonics = Harmonics::High;
				}else{
					m_harmonics = Harmonics::Low;
				}
				new_bots.push_back(b);
				pos2cmd.emplace(new_bots.back().pos, make_backward_command(c));
			}else if(c.type == detail::CommandType::SMove){
				const auto& p = c.u.smove;
				new_bots.push_back(detail::Bot{ b.bid, b.pos + p.lld, b.seeds });
				pos2cmd.emplace(new_bots.back().pos, make_backward_command(c));
			}else if(c.type == detail::CommandType::LMove){
				const auto& p = c.u.lmove;
				new_bots.push_back(detail::Bot{ b.bid, b.pos + p.sld1 + p.sld2, b.seeds });
				pos2cmd.emplace(new_bots.back().pos, make_backward_command(c));
			}else if(c.type == detail::CommandType::Fission){
				const auto& p = c.u.fission;
				const auto ss = detail::split_seeds(b.seeds, p.m);
				new_bots.push_back(detail::Bot{ b.bid, b.pos, ss.second });
				const auto bwd_p = make_backward_command(c);
				pos2cmd.emplace(new_bots.back().pos, bwd_p);
				const auto new_bid = __builtin_ctz(ss.first);
				new_bots.push_back(detail::Bot{ new_bid, b.pos + p.nd, ss.first ^ (1u << new_bid) });
				auto bwd_s = bwd_p;
				bwd_s.type = detail::CommandType::FusionS;
				bwd_s.u.fusion.nd = -bwd_p.u.fusion.nd;
				pos2cmd.emplace(new_bots.back().pos, bwd_s);
			}else if(c.type == detail::CommandType::Fill){
				const auto& p = c.u.fill;
				const auto pos = b.pos + p.nd;
				m_matrix(pos.z, pos.y, pos.x) = 0;
				new_bots.push_back(b);
				pos2cmd.emplace(new_bots.back().pos, make_backward_command(c));
			}else if(c.type == detail::CommandType::FusionP){
				const auto& p = c.u.fill;
				const auto it = pos2idx.find(b.pos + p.nd);
				if(it == pos2idx.end()){ throw std::runtime_error("unmatched fusion"); }
				const auto& s = m_bots[it->second];
				new_bots.push_back(detail::Bot{ b.bid, b.pos, b.seeds | s.seeds | (1u << s.bid) });
				auto bwd_c = make_backward_command(c);
				bwd_c.u.fission.m = __builtin_popcount(s.seeds);
				pos2cmd.emplace(new_bots.back().pos, make_backward_command(c));
			}else if(c.type == detail::CommandType::FusionS){
				// TODO validation
			}
		}

		std::vector<detail::Command> new_pending_commands(new_bots.size());
		for(auto& c : new_pending_commands){ c.type = detail::CommandType::Empty; }
		m_bots = std::move(new_bots);
		m_pending_commands = std::move(new_pending_commands);
		m_pos2commands.emplace_back(std::move(pos2cmd));
	}

	void export_trace(const std::string& filename) const {
		State s(m_matrix.r(), m_initial_bid, m_initial_seeds);
		for(int i = m_pos2commands.size() - 2; i >= 0; --i){
			for(int j = 0; j < s.num_bots(); ++j){
				const auto it = m_pos2commands[i].find(s.bots(j).pos());
				s.bots(j).exec(it->second);
			}
			s.commit();
		}
		s.bots(0).halt();
		s.commit();
		s.export_trace(filename);
	}

};

std::ostream& operator<<(std::ostream& os, const BackwardState::ConstBotReference& r){
	// TODO format seeds
	return os << "Bot{ bid=" << r.bid() << ", pos=" << r.pos() << ", seeds=" << r.seeds() << " }";
}
std::ostream& operator<<(std::ostream& os, const BackwardState::BotReference& r){
	return os << "Bot{ bid=" << r.bid() << ", pos=" << r.pos() << ", seeds=" << r.seeds() << " }";
}

#endif
