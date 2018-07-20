#ifndef NBTASM_H
#define NBTASM_H

#include <iostream>
#include <tuple>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>

enum class Direction { X, Y, Z };
enum class Harmonics { Low, High };

struct Vec3 {
	int x, y, z;

	bool operator==(const Vec3& v) const { return x == v.x && y == v.y && z == v.z; }
	bool operator!=(const Vec3& v) const { return !(*this == v); }

	bool operator< (const Vec3& v) const { return std::make_tuple(x, y, z) <  std::make_tuple(v.x, v.y, v.z); }
	bool operator<=(const Vec3& v) const { return std::make_tuple(x, y, z) <= std::make_tuple(v.x, v.y, v.z); }
	bool operator> (const Vec3& v) const { return std::make_tuple(x, y, z) >  std::make_tuple(v.x, v.y, v.z); }
	bool operator>=(const Vec3& v) const { return std::make_tuple(x, y, z) >= std::make_tuple(v.x, v.y, v.z); }

	Vec3 operator-() const { return Vec3{ -x, -y, -z }; }
	Vec3 operator+(const Vec3& v) const { return Vec3{ x + v.x, y + v.y, z + v.z }; }
	Vec3 operator-(const Vec3& v) const { return Vec3{ x - v.x, y - v.y, z - v.z }; }
	Vec3& operator+=(const Vec3& v){ x += v.x; y += v.y; z += v.z; return *this; }
	Vec3& operator-=(const Vec3& v){ x -= v.x; y -= v.y; z -= v.z; return *this; }
};

std::ostream& operator<<(std::ostream& os, const Vec3& v){
	return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}


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

	bool test(const Vec3& a, const Vec3& b) const {
		const int i_lo = std::min(a.z, b.z), i_hi = std::max(a.z, b.z);
		const int j_lo = std::min(a.z, b.z), j_hi = std::max(a.z, b.z);
		const int k_lo = std::min(a.z, b.z), k_hi = std::max(a.z, b.z);
		for(int i = i_lo; i <= i_hi; ++i){
			for(int j = j_lo; j <= j_hi; ++j){
				for(int k = k_lo; k <= k_hi; ++k){
					if((*this)(i, j, k)){ return false; }
				}
			}
		}
		return true;
	}

};


namespace detail {

enum class CommandType {
	Empty,
	Halt,
	Wait,
	Flip,
	SMove,
	LMove,
	Fission,
	Fill,
	FusionP,
	FusionS
};

struct SMoveParams { Vec3 lld; };
struct LMoveParams { Vec3 sld1, sld2; };
struct FissionParams { Vec3 nd; int m; };
struct FillParams { Vec3 nd; };
struct FusionParams { Vec3 nd; };

struct Command {
	CommandType type;
	union {
		SMoveParams smove;
		LMoveParams lmove;
		FissionParams fission;
		FillParams fill;
		FusionParams fusion;
	} u;
};

struct Bot {
	int bid;
	Vec3 pos;
	uint32_t seeds;
};

static inline std::pair<uint32_t, uint32_t> split_seeds(uint32_t s, int m){
	for(int i = 0, k = 0; i < 8 * sizeof(s); ++i){
		if(k == m){
			const uint32_t mask = (1u << i) - 1u;
			return std::make_pair(s & mask, s & ~mask);
		}
		if(s & (1 << i)){ ++k; }
	}
	return std::make_pair(s, 0);
}

static inline bool test_near_distance(const Vec3& v){
	if(v.x >= 2 || v.y >= 2 || v.z >= 2){ return false; }
	if(abs(v.x) + abs(v.y) + abs(v.z) > 2){ return false; }
	return true;
}
static inline void throw_test_near_distance(const Vec3& v){
	if(!test_near_distance(v)){ throw std::runtime_error("invalid nd"); }
}

static inline bool test_short_distance(const Vec3& v){
	if(v.y == 0 && v.z == 0){ return -5 <= v.x && v.x <= 5; }
	if(v.x == 0 && v.z == 0){ return -5 <= v.y && v.y <= 5; }
	if(v.x == 0 && v.y == 0){ return -5 <= v.z && v.z <= 5; }
	return false;
}
static inline void throw_test_short_distance(const Vec3& v){
	if(!test_short_distance(v)){ throw std::runtime_error("invalid sld"); }
}

static inline bool test_long_distance(const Vec3& v){
	if(v.y == 0 && v.z == 0){ return -15 <= v.x && v.x <= 15; }
	if(v.x == 0 && v.z == 0){ return -15 <= v.y && v.y <= 15; }
	if(v.x == 0 && v.y == 0){ return -15 <= v.z && v.z <= 15; }
	return false;
}
static inline void throw_test_long_distance(const Vec3& v){
	if(!test_long_distance(v)){ throw std::runtime_error("invalid lld"); }
}

static inline bool test_position_range(int r, const Vec3& v){
	return (0 <= v.x && v.x < r) && (0 <= v.y && v.y < r) && (0 <= v.z && v.z < r);
}
static inline void throw_test_position_range(int r, const Vec3& v){
	if(!test_position_range(r, v)){ throw std::runtime_error("position out of range"); }
}

static inline unsigned int encode_near_distance(const Vec3& v){
	return (v.x + 1) * 9 + (v.y + 1) * 3 + (v.z + 1);
}
static inline unsigned int encode_short_distance(const Vec3& v){
	if(v.y == 0 && v.z == 0){ return 0x10 | (v.x + 5); }
	if(v.x == 0 && v.z == 0){ return 0x20 | (v.y + 5); }
	if(v.x == 0 && v.y == 0){ return 0x30 | (v.z + 5); }
	return 0;
}
static inline unsigned int encode_long_distance(const Vec3& v){
	if(v.y == 0 && v.z == 0){ return 0x20 | (v.x + 15); }
	if(v.x == 0 && v.z == 0){ return 0x40 | (v.y + 15); }
	if(v.x == 0 && v.y == 0){ return 0x60 | (v.z + 15); }
	return 0;
}

}



class State {

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
		State& m_state;
		detail::Bot& m_bot;

		detail::Command& m_command;

	public:
		BotReference(State& s, detail::Bot& b, detail::Command& c)
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
		void fill(const Vec3& nd){
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

	};


private:
	int64_t m_energy;
	Harmonics m_harmonics;
	VoxelGrid m_matrix;
	std::vector<detail::Bot> m_bots;
	std::vector<detail::Command> m_trace;

	std::vector<detail::Command> m_pending_commands;

public:
	State()
		: m_energy(0)
		, m_harmonics(Harmonics::Low)
		, m_matrix()
		, m_bots()
		, m_trace()
		, m_pending_commands()
	{ }

	explicit State(int r, int bid = 1, uint32_t seeds = 0x1ffffcu)
		: m_energy(0)
		, m_harmonics(Harmonics::Low)
		, m_matrix(r)
		, m_bots(1, detail::Bot{ bid, Vec3{ 0, 0, 0 }, seeds })
		, m_trace()
		, m_pending_commands(1)
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
		for(size_t i = 0; i < m_bots.size(); ++i){
			const auto& b = m_bots[i];
			const auto& c = m_pending_commands[i];
			if(c.type == detail::CommandType::Empty){
				throw std::runtime_error("no commands are supplied");
			}else if(c.type == detail::CommandType::Halt){
				if(m_bots.size() != 1){ throw std::runtime_error("constraints is not satisfied for halt"); }
				new_bots.push_back(b);
			}else if(c.type == detail::CommandType::Wait){
				new_bots.push_back(b);
			}else if(c.type == detail::CommandType::Flip){
				if(m_harmonics == Harmonics::Low){
					m_harmonics = Harmonics::High;
				}else{
					m_harmonics = Harmonics::Low;
				}
				new_bots.push_back(b);
			}else if(c.type == detail::CommandType::SMove){
				const auto& p = c.u.smove;
				new_bots.push_back(detail::Bot{ b.bid, b.pos + p.lld, b.seeds });
			}else if(c.type == detail::CommandType::LMove){
				const auto& p = c.u.lmove;
				new_bots.push_back(detail::Bot{ b.bid, b.pos + p.sld1 + p.sld2, b.seeds });
			}else if(c.type == detail::CommandType::Fission){
				const auto& p = c.u.fission;
				const auto ss = detail::split_seeds(b.seeds, p.m);
				new_bots.push_back(detail::Bot{ b.bid, b.pos, ss.second });
				const auto new_bid = __builtin_ctz(ss.first);
				new_bots.push_back(detail::Bot{ new_bid, b.pos + p.nd, ss.first ^ (1u << new_bid) });
			}else if(c.type == detail::CommandType::Fill){
				const auto& p = c.u.fill;
				const auto pos = b.pos + p.nd;
				m_matrix(pos.z, pos.y, pos.x) = 1;
				new_bots.push_back(b);
			}else if(c.type == detail::CommandType::FusionP){
				const auto& p = c.u.fill;
				const auto it = pos2idx.find(b.pos + p.nd);
				if(it == pos2idx.end()){ throw std::runtime_error("unmatched fusion"); }
				const auto& s = m_bots[it->second];
				new_bots.push_back(detail::Bot{ b.bid, b.pos, b.seeds | s.seeds | (1u << s.bid) });
			}else if(c.type == detail::CommandType::FusionS){
				// TODO validation
			}
		}

		using pair_type = std::pair<int, detail::Command>;
		std::vector<pair_type> command_buffer;
		for(size_t i = 0; i < m_bots.size(); ++i){
			command_buffer.emplace_back(m_bots[i].bid, m_pending_commands[i]);
		}
		std::sort(
			command_buffer.begin(), command_buffer.end(),
			[](const pair_type& a, const pair_type& b){
				return a.first < b.first;
			});
		for(const auto& p : command_buffer){
			m_trace.push_back(p.second);
		}

		std::vector<detail::Command> new_pending_commands(new_bots.size());
		for(auto& c : new_pending_commands){ c.type = detail::CommandType::Empty; }
		m_bots = std::move(new_bots);
		m_pending_commands = std::move(new_pending_commands);
	}

	void export_trace(const std::string& filename) const {
		auto fp = fopen(filename.c_str(), "wb");
		for(const auto& c : m_trace){
			if(c.type == detail::CommandType::Halt){
				fputc(0xff, fp);
			}else if(c.type == detail::CommandType::Wait){
				fputc(0xfe, fp);
			}else if(c.type == detail::CommandType::Flip){
				fputc(0xfd, fp);
			}else if(c.type == detail::CommandType::SMove){
				const auto& p = c.u.smove;
				const auto lld = detail::encode_long_distance(p.lld);
				fputc(0x04 | ((lld >> 5) << 4), fp);
				fputc(lld & 0x1f, fp);
			}else if(c.type == detail::CommandType::LMove){
				const auto& p = c.u.lmove;
				const auto sld1 = detail::encode_short_distance(p.sld1);
				const auto sld2 = detail::encode_short_distance(p.sld2);
				fputc(0x0c | ((sld2 >> 4) << 6) | ((sld1 >> 4) << 4), fp);
				fputc(((sld2 & 0x0f) << 4) | (sld1 & 0x0f), fp);
			}else if(c.type == detail::CommandType::FusionP){
				const auto& p = c.u.fusion;
				fputc(0x07 | (detail::encode_near_distance(p.nd) << 3), fp);
			}else if(c.type == detail::CommandType::FusionS){
				const auto& p = c.u.fusion;
				fputc(0x06 | (detail::encode_near_distance(p.nd) << 3), fp);
			}else if(c.type == detail::CommandType::Fission){
				const auto& p = c.u.fission;
				fputc(0x05 | (detail::encode_near_distance(p.nd) << 3), fp);
				fputc(p.m, fp);
			}else if(c.type == detail::CommandType::Fill){
				const auto& p = c.u.fill;
				fputc(0x03 | (detail::encode_near_distance(p.nd) << 3), fp);
			}
		}
		fclose(fp);
	}

};

std::ostream& operator<<(std::ostream& os, const State::ConstBotReference& r){
	// TODO format seeds
	return os << "Bot{ bid=" << r.bid() << ", pos=" << r.pos() << ", seeds=" << r.seeds() << " }";
}
std::ostream& operator<<(std::ostream& os, const State::BotReference& r){
	return os << "Bot{ bid=" << r.bid() << ", pos=" << r.pos() << ", seeds=" << r.seeds() << " }";
}

#endif
