#ifndef NBTASM_H
#define NBTASM_H

#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <iomanip>

enum class Harmonics { Low, High };

static inline Harmonics flip(Harmonics h){
	return h == Harmonics::Low ? Harmonics::High : Harmonics::Low;
}


struct Vec3 {
	int x, y, z;

	Vec3() : x(0), y(0), z(0) { }
	Vec3(int x, int y, int z) : x(x), y(y), z(z) { }

	bool operator==(const Vec3& v) const noexcept { return x == v.x && y == v.y && z == v.z; }
	bool operator!=(const Vec3& v) const noexcept { return !(*this == v); }

	bool operator< (const Vec3& v) const noexcept { return std::make_tuple(x, y, z) <  std::make_tuple(v.x, v.y, v.z); }
	bool operator<=(const Vec3& v) const noexcept { return std::make_tuple(x, y, z) <= std::make_tuple(v.x, v.y, v.z); }
	bool operator> (const Vec3& v) const noexcept { return std::make_tuple(x, y, z) >  std::make_tuple(v.x, v.y, v.z); }
	bool operator>=(const Vec3& v) const noexcept { return std::make_tuple(x, y, z) >= std::make_tuple(v.x, v.y, v.z); }

	Vec3 operator-() const noexcept { return Vec3{ -x, -y, -z }; }
	Vec3 operator+(const Vec3& v) const noexcept { return Vec3{ x + v.x, y + v.y, z + v.z }; }
	Vec3 operator-(const Vec3& v) const noexcept { return Vec3{ x - v.x, y - v.y, z - v.z }; }
	Vec3& operator+=(const Vec3& v) noexcept { x += v.x; y += v.y; z += v.z; return *this; }
	Vec3& operator-=(const Vec3& v) noexcept { x -= v.x; y -= v.y; z -= v.z; return *this; }

	int mlen() const noexcept { return std::abs(x) + std::abs(y) + std::abs(z); }

	unsigned int encode_near_distance() const noexcept {
		return (x + 1) * 9 + (y + 1) * 3 + (z + 1);
	}
	unsigned int encode_short_distance() const noexcept {
		if(y == 0 && z == 0){ return 0x10 | (x + 5); }
		if(x == 0 && z == 0){ return 0x20 | (y + 5); }
		if(x == 0 && y == 0){ return 0x30 | (z + 5); }
		return 0;
	}
	unsigned int encode_long_distance() const noexcept {
		if(y == 0 && z == 0){ return 0x20 | (x + 15); }
		if(x == 0 && z == 0){ return 0x40 | (y + 15); }
		if(x == 0 && y == 0){ return 0x60 | (z + 15); }
		return 0;
	}

  int abs() const {
    return std::abs(x) + std::abs(y) + std::abs(z);
  }

  int encode(const int r) const {
    return z * r * r + y * r + x;
  }

};

inline std::ostream& operator<<(std::ostream& os, const Vec3& v){
	return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

namespace std {
	template <> struct hash<Vec3> {
		using argument_type = Vec3;
		using result_type = std::size_t;
		result_type operator()(const argument_type& v) const noexcept {
			result_type h = 0;
			h = h * 1234567891u + static_cast<result_type>(v.x);
			h = h * 1234567891u + static_cast<result_type>(v.y);
			h = h * 1234567891u + static_cast<result_type>(v.z);
			return h;
		}
	};
}


class VoxelGrid {

private:
	int m_r;
	std::vector<uint8_t> m_grid;

public:
	VoxelGrid() noexcept
		: m_grid()
	{ }

	explicit VoxelGrid(int r)
		: m_grid(r * r * r)
		, m_r(r)
	{ }

	uint8_t operator()(int i, int j, int k) const noexcept {
		const int r = m_r;
		return m_grid[i * r * r + j * r + k];
	}
	uint8_t& operator()(int i, int j, int k) noexcept {
		const int r = m_r;
		return m_grid[i * r * r + j * r + k];
	}

	uint8_t operator()(const Vec3 &v) const {
		const int r = m_r;
		return m_grid[v.z * r * r + v.y * r + v.x];
	}
	uint8_t& operator()(const Vec3 &v){
		const int r = m_r;
		return m_grid[v.z * r * r + v.y * r + v.x];
	}

	int r() const noexcept { return m_r; }
	int size() const noexcept { return m_r; }

	bool test(const Vec3& a, const Vec3& b) const {
		const int i_lo = std::min(a.z, b.z), i_hi = std::max(a.z, b.z);
		const int j_lo = std::min(a.y, b.y), j_hi = std::max(a.y, b.y);
		const int k_lo = std::min(a.x, b.x), k_hi = std::max(a.x, b.x);
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

std::ostream &operator<<(std::ostream &out, const VoxelGrid &grid) {
  const int r = grid.r();

  out << r << std::endl;
  for (int y = 0; y < r; y++) {
    for (int z = 0; z < r; z++) {
      for (int x = 0; x < r; x++) {
        out << std::setw(5) << static_cast<int>(grid(z, y, x));
      }
      out << std::endl;
    }
    out << "---" << std::endl;
  }
  return out;
}

enum class CommandType {
	Halt,
	Wait,
	Flip,
	SMove,
	LMove,
	Fission,
	Fill,
	Empty,
	FusionP,
	FusionS
};

struct Command {
	CommandType type;
	Vec3 v1, v2;
	int m;

	Command() noexcept : type(CommandType::Wait) { }
	explicit Command(CommandType t) noexcept : type(t) { }

	const Vec3& smove_lld() const noexcept { return v1; }
	const Vec3& lmove_sld1() const noexcept { return v1; }
	const Vec3& lmove_sld2() const noexcept { return v2; }
	const Vec3& fission_nd() const noexcept { return v1; }
	int fission_m() const noexcept { return m; }
	const Vec3& fill_nd() const noexcept { return v1; }
	const Vec3& empty_nd() const noexcept { return v1; }
	const Vec3& fusion_nd() const noexcept { return v1; }

	Vec3& smove_lld() noexcept { return v1; }
	Vec3& lmove_sld1() noexcept { return v1; }
	Vec3& lmove_sld2() noexcept { return v2; }
	Vec3& fission_nd() noexcept { return v1; }
	Vec3& fill_nd() noexcept { return v1; }
	Vec3& empty_nd() noexcept { return v1; }
	Vec3& fusion_nd() noexcept { return v1; }

	Command& smove_lld(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& lmove_sld1(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& lmove_sld2(const Vec3& v) noexcept { v2 = v; return *this; }
	Command& fission_nd(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& fission_m(int v) noexcept { m = v; return *this; }
	Command& fill_nd(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& empty_nd(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& fusion_nd(const Vec3& v) noexcept { v1 = v; return *this; }

	void write_binary(std::ostream& os) const {
		if(type == CommandType::Halt){
			os.put(0xff);
		}else if(type == CommandType::Wait){
			os.put(0xfe);
		}else if(type == CommandType::Flip){
			os.put(0xfd);
		}else if(type == CommandType::SMove){
			const auto lld = smove_lld().encode_long_distance();
			os.put(0x04 | ((lld >> 5) << 4));
			os.put(lld & 0x1f);
		}else if(type == CommandType::LMove){
			const auto sld1 = lmove_sld1().encode_short_distance();
			const auto sld2 = lmove_sld2().encode_short_distance();
			os.put(0x0c | ((sld2 >> 4) << 6) | ((sld1 >> 4) << 4));
			os.put(((sld2 & 0x0f) << 4) | (sld1 & 0x0f));
		}else if(type == CommandType::FusionP){
			os.put(0x07 | (fusion_nd().encode_near_distance() << 3));
		}else if(type == CommandType::FusionS){
			os.put(0x06 | (fusion_nd().encode_near_distance() << 3));
		}else if(type == CommandType::Fission){
			os.put(0x05 | (fission_nd().encode_near_distance() << 3));
			os.put(fission_m());
		}else if(type == CommandType::Fill){
			os.put(0x03 | (fill_nd().encode_near_distance() << 3));
		}
	}
};

inline std::ostream& operator<<(std::ostream& os, const Command& c){
	os << "Command{ ";
	if(c.type == CommandType::Empty){
		os << "type=Empty";
	}else if(c.type == CommandType::Halt){
		os << "type=Halt";
	}else if(c.type == CommandType::Wait){
		os << "type=Wait";
	}else if(c.type == CommandType::Flip){
		os << "type=Flip";
	}else if(c.type == CommandType::SMove){
		os << "type=SMove, ldd1=" << c.smove_lld();
	}else if(c.type == CommandType::LMove){
		os << "type=LMove, sld1=" << c.lmove_sld1() << ", sld2=" << c.lmove_sld2();
	}else if(c.type == CommandType::Fission){
		os << "type=Fission, nd=" << c.fission_nd() << ", m=" << c.fission_m();
	}else if(c.type == CommandType::Fill){
		os << "type=Fill, nd=" << c.fill_nd();
	}else if(c.type == CommandType::Empty){
		os << "type=Empty, nd=" << c.fill_nd();
	}else if(c.type == CommandType::FusionP){
		os << "type=FusionP, nd=" << c.fusion_nd();
	}else if(c.type == CommandType::FusionS){
		os << "type=FusionS, nd=" << c.fusion_nd();
	}
	return os << " }";
}


namespace detail {

static inline std::pair<uint32_t, uint32_t> split_seeds(uint32_t s, int m) noexcept {
	for(int i = 0, k = 0; i < 8 * sizeof(s); ++i){
		if(k == m){
			const uint32_t mask = (1u << i) - 1u;
			return std::make_pair(s & mask, s & ~mask);
		}
		if(s & (1 << i)){ ++k; }
	}
	return std::make_pair(s, 0);
}

static inline void format_seeds(std::ostream& os, uint32_t s){
	const int len = 8 * sizeof(s);
	bool is_first = true;
	os << "(";
	for(int head = 0; head < len; ){
		if(!(s & (1 << head))){
			++head;
			continue;
		}
		int tail = head;
		while(tail < len){
			if(!(s & (1 << tail))){ break; }
			++tail;
		}
		if(!is_first){ os << ", "; }
		is_first = false;
		if(tail - head == 1){
			os << head;
		}else if(tail - head == 2){
			os << head << ", " << head + 1;
		}else{
			os << head << ".." << tail - 1;
		}
		head = tail;
	}
	os << ")";
}

static inline bool test_near_distance(const Vec3& v) noexcept {
	if(abs(v.x) >= 2 || abs(v.y) >= 2 || abs(v.z) >= 2){ return false; }
	if(abs(v.x) + abs(v.y) + abs(v.z) > 2){ return false; }
	return true;
}
static inline void throw_test_near_distance(const Vec3& v){
	if(!test_near_distance(v)){ throw std::runtime_error("invalid nd"); }
}

static inline bool test_short_distance(const Vec3& v) noexcept {
	if(v.y == 0 && v.z == 0){ return -5 <= v.x && v.x <= 5; }
	if(v.x == 0 && v.z == 0){ return -5 <= v.y && v.y <= 5; }
	if(v.x == 0 && v.y == 0){ return -5 <= v.z && v.z <= 5; }
	return false;
}
static inline void throw_test_short_distance(const Vec3& v){
	if(!test_short_distance(v)){ throw std::runtime_error("invalid sld"); }
}

static inline bool test_long_distance(const Vec3& v) noexcept {
	if(v.y == 0 && v.z == 0){ return -15 <= v.x && v.x <= 15; }
	if(v.x == 0 && v.z == 0){ return -15 <= v.y && v.y <= 15; }
	if(v.x == 0 && v.y == 0){ return -15 <= v.z && v.z <= 15; }
	return false;
}
static inline void throw_test_long_distance(const Vec3& v){
	if(!test_long_distance(v)){ throw std::runtime_error("invalid lld"); }
}

static inline bool test_position_range(int r, const Vec3& v) noexcept {
	return (0 <= v.x && v.x < r) && (0 <= v.y && v.y < r) && (0 <= v.z && v.z < r);
}
static inline void throw_test_position_range(int r, const Vec3& v){
	if(!test_position_range(r, v)){ throw std::runtime_error("position out of range"); }
}

}


class State {

private:
	struct Bot {
		int bid;
		Vec3 pos;
		uint32_t seeds;
	};

public:
	class ConstBotReference {
	private:
		const Bot& m_bot;
	public:
		explicit ConstBotReference(const Bot& b) noexcept
			: m_bot(b)
		{ }
		int bid() const noexcept { return m_bot.bid; }
		const Vec3& pos() const noexcept { return m_bot.pos; }
		uint32_t seeds() const noexcept { return m_bot.seeds; }
	};


	class BotReference {

	private:
		State& m_state;
		Bot& m_bot;
		Command& m_command;

	public:
		BotReference(State& s, Bot& b, Command& c) noexcept
			: m_state(s)
			, m_bot(b)
			, m_command(c)
		{ }

		int bid() const noexcept { return m_bot.bid; }
		const Vec3& pos() const noexcept { return m_bot.pos; }
		uint32_t seeds() const noexcept { return m_bot.seeds; }

		void halt(){ m_command = Command(CommandType::Halt); }
		void wait(){ m_command = Command(CommandType::Wait); }
		void flip(){ m_command = Command(CommandType::Flip); }
		void smove(const Vec3& lld){
			detail::throw_test_long_distance(lld);
			detail::throw_test_position_range(m_state.matrix().size(), m_bot.pos + lld);
			m_command = Command(CommandType::SMove)
				.smove_lld(lld);
		}
		void lmove(const Vec3& sld1, const Vec3& sld2){
			detail::throw_test_short_distance(sld1);
			detail::throw_test_short_distance(sld2);
			detail::throw_test_position_range(m_state.matrix().r(), m_bot.pos + sld1 + sld2);
			m_command = Command(CommandType::LMove)
				.lmove_sld1(sld1)
				.lmove_sld2(sld2);
		}
		int fission(const Vec3& nd, int m){
			detail::throw_test_near_distance(nd);
			const int n = __builtin_popcount(m_bot.seeds);
			if(m < 0 || n < m + 1){ throw std::runtime_error("fission: invalid m"); }
			m_command = Command(CommandType::Fission)
				.fission_nd(nd)
				.fission_m(m);
			return __builtin_ctz(m_bot.seeds);
		}
		void fill(const Vec3& nd){
			detail::throw_test_near_distance(nd);
			m_command = Command(CommandType::Fill)
				.fill_nd(nd);
		}
		void empty(const Vec3& nd){
			detail::throw_test_near_distance(nd);
			m_command = Command(CommandType::Empty)
				.empty_nd(nd);
		}
		void fusion_p(const Vec3& nd){
			detail::throw_test_near_distance(nd);
			m_command = Command(CommandType::FusionP)
				.fusion_nd(nd);
		}
		void fusion_s(const Vec3& nd){
			detail::throw_test_near_distance(nd);
			m_command = Command(CommandType::FusionS)
				.fusion_nd(nd);
		}

		void exec(const Command& cmd){
			m_command = cmd;
		}
	};


private:
	int64_t m_energy;
	Harmonics m_harmonics;
	VoxelGrid m_matrix;
	std::vector<Bot> m_bots;
	std::vector<Command> m_trace;

	std::vector<Command> m_pending_commands;

	int m_max_num_bots;

public:
	State()
		: m_energy(0)
		, m_harmonics(Harmonics::Low)
		, m_matrix()
		, m_bots()
		, m_trace()
		, m_pending_commands()
		, m_max_num_bots(0)
	{ }

	State(int size, int max_num_bots)
		: m_energy(0)
		, m_harmonics(Harmonics::Low)
		, m_matrix(size)
		, m_bots(1, Bot{ 1, Vec3(0, 0, 0), (((1u << (max_num_bots - 1)) - 1) << 2) })
		, m_trace()
		, m_pending_commands(1)
		, m_max_num_bots(max_num_bots)
	{ }

	State(const VoxelGrid& initial, int max_num_bots)
		: m_energy(0)
		, m_harmonics(Harmonics::Low)
		, m_matrix(initial)
		, m_bots(1, Bot{ 1, Vec3(0, 0, 0), (((1u << (max_num_bots - 1)) - 1) << 2) })
		, m_trace()
		, m_pending_commands(1)
		, m_max_num_bots(max_num_bots)
	{ }

	int energy() const noexcept { return m_energy; }
	Harmonics harmonics() const noexcept { return m_harmonics; }

	const VoxelGrid& matrix() const noexcept { return m_matrix; }
	uint8_t  matrix(int i, int j, int k) const noexcept { return m_matrix(i, j, k); }
	uint8_t& matrix(int i, int j, int k)       noexcept { return m_matrix(i, j, k); }

	ConstBotReference bots(size_t i) const noexcept { return ConstBotReference(m_bots[i]); }
	BotReference bots(size_t i) noexcept { return BotReference(*this, m_bots[i], m_pending_commands[i]); }

	ConstBotReference from_bid(int bid) const {
		for(size_t i = 0; i < m_bots.size(); ++i){
			if(m_bots[i].bid == bid){ return ConstBotReference(m_bots[i]); }
		}
		throw std::runtime_error("bot not found");
	}
	BotReference from_bid(int bid){
		for(size_t i = 0; i < m_bots.size(); ++i){
			if(m_bots[i].bid == bid){ return BotReference(*this, m_bots[i], m_pending_commands[i]); }
		}
		throw std::runtime_error("bot not found");
	}

	size_t num_bots() const { return m_bots.size(); }
	size_t max_num_bots() const { return m_max_num_bots; }

	const std::vector<Command>& trace() const noexcept { return m_trace; }

	void commit(){
		// build map to get bot from position for fusion
		std::unordered_map<Vec3, size_t> pos2idx;
		for(size_t i = 0; i < m_bots.size(); ++i){
			pos2idx.emplace(m_bots[i].pos, i);
		}
		// process commands: create new bot list
		std::vector<Bot> new_bots;
		for(size_t i = 0; i < m_bots.size(); ++i){
			const auto& b = m_bots[i];
			const auto& c = m_pending_commands[i];
			if(c.type == CommandType::Wait){
				new_bots.push_back(b);
			}else if(c.type == CommandType::Flip){
				new_bots.push_back(b);
			}else if(c.type == CommandType::SMove){
				new_bots.push_back(Bot{ b.bid, b.pos + c.smove_lld(), b.seeds });
			}else if(c.type == CommandType::LMove){
				new_bots.push_back(Bot{ b.bid, b.pos + c.lmove_sld1() + c.lmove_sld2(), b.seeds });
			}else if(c.type == CommandType::Fission){
				const auto sp = detail::split_seeds(b.seeds, c.fission_m());
				new_bots.push_back(Bot{ b.bid, b.pos, sp.first });
				const int new_bid = __builtin_ctz(sp.second);
				new_bots.push_back(Bot{ new_bid, b.pos + c.fission_nd(), sp.second & ~(1u << new_bid) });
			}else if(c.type == CommandType::Fill){
				new_bots.push_back(b);
			}else if(c.type == CommandType::Empty){
				new_bots.push_back(b);
			}else if(c.type == CommandType::FusionP){
				const auto it = pos2idx.find(b.pos + c.fusion_nd());
				if(it == pos2idx.end()){ throw std::runtime_error("unmatched fusion"); }
				const auto& secondary = m_bots[it->second];
				new_bots.push_back(Bot{ b.bid, b.pos, b.seeds | secondary.seeds | (1u << secondary.bid) });
			}else if(c.type == CommandType::FusionS){
				const auto it = pos2idx.find(b.pos + c.fusion_nd());
				if(it == pos2idx.end()){ throw std::runtime_error("unmatched fusion"); }
			}
		}
		// sort by bid
		std::sort(
			new_bots.begin(), new_bots.end(),
			[](const Bot& a, const Bot& b){ return a.bid < b.bid; });
		// allocate next command buffer
		std::vector<Command> new_pending_commands(new_bots.size());
		// update trace
		m_trace.resize(m_trace.size() + m_bots.size());
		for(size_t i = 0; i < m_bots.size(); ++i){
			m_trace[m_trace.size() - m_bots.size() + i] = m_pending_commands[i];
		}
		// update global state
		for(size_t i = 0; i < m_bots.size(); ++i){
			const auto& b = m_bots[i];
			const auto& c = m_pending_commands[i];
			if(c.type == CommandType::Flip){
				m_harmonics = flip(m_harmonics);
			}else if(c.type == CommandType::SMove){
				m_energy += 2 * c.smove_lld().mlen();
			}else if(c.type == CommandType::LMove){
				m_energy += 2 * (c.lmove_sld1().mlen() + 2 + c.lmove_sld2().mlen());
			}else if(c.type == CommandType::Fission){
				m_energy += 24;
			}else if(c.type == CommandType::Fill){
				const auto p = b.pos + c.fill_nd();
				if(m_matrix(p.z, p.y, p.x)){
					m_energy += 6;
				}else{
					m_matrix(p.z, p.y, p.x) = 1;
					m_energy += 12;
				}
			}else if(c.type == CommandType::Empty){
				const auto p = b.pos + c.empty_nd();
				if(m_matrix(p.z, p.y, p.x)){
					m_matrix(p.z, p.y, p.x) = 0;
					m_energy += 12;
				}else{
					m_energy += 6;
				}
			}else if(c.type == CommandType::FusionP){
				m_energy -= 24;
			}
		}
		const int64_t r = m_matrix.size();
		m_energy += (m_harmonics == Harmonics::High ? 30 : 3) * r * r * r;
		m_energy += 20 * new_bots.size();
		// update bots
		m_bots = std::move(new_bots);
		m_pending_commands = std::move(new_pending_commands);
	}

	void export_trace(const std::string& filename) const {
		std::ofstream ofs(filename.c_str(), std::ios::out | std::ios::binary);
		for(const auto& c : m_trace){
			c.write_binary(ofs);
		}
		ofs.close();
	}

};

std::ostream& operator<<(std::ostream& os, const State::ConstBotReference& r){
	os << "Bot{ bid=" << r.bid() << ", pos=" << r.pos() << ", seeds=";
	detail::format_seeds(os, r.seeds());
	return os << " }";
}
std::ostream& operator<<(std::ostream& os, const State::BotReference& r){
	os << "Bot{ bid=" << r.bid() << ", pos=" << r.pos() << ", seeds=";
	detail::format_seeds(os, r.seeds());
	return os << " }";
}

#endif
