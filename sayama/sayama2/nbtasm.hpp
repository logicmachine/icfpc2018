#ifndef NBTASM_H
#define NBTASM_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <cstdint>


enum class Harmonics { Low, High };

static inline Harmonics flip(Harmonics h){
	return h == Harmonics::Low ? Harmonics::High : Harmonics::Low;
}


struct Vec3 {
	int x, y, z;

	Vec3() : x(0), y(0), z(0) { }
	Vec3(int x, int y, int z) : x(x), y(y), z(z) { }

	bool operator==(const Vec3& v) const noexcept { return x == v.x && y == v.y && z == v.z; } bool operator!=(const Vec3& v) const noexcept { return !(*this == v); }

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
	unsigned int encode_far_distance() const noexcept {
		return (x + 30) | ((y + 30) << 8) | ((z + 30) << 16);
	}

  int abs() const {
    return std::abs(x) + std::abs(y) + std::abs(z);
  }

  int encode(const int r) const {
    return z * r * r + y * r + x;
  }

	static Vec3 decode_near_distance(unsigned int x) noexcept {
		return Vec3((x / 9) - 1, (x / 3 % 3) - 1, (x % 3) - 1);
	}
	static Vec3 decode_short_distance(unsigned int x) noexcept {
		if((x >> 4) == 0x01){ return Vec3((x & 0x0f) - 5, 0, 0); }
		if((x >> 4) == 0x02){ return Vec3(0, (x & 0x0f) - 5, 0); }
		if((x >> 4) == 0x03){ return Vec3(0, 0, (x & 0x0f) - 5); }
		return Vec3();
	}
	static Vec3 decode_long_distance(unsigned int x) noexcept {
		if((x >> 5) == 0x01){ return Vec3((x & 0x1f) - 15, 0, 0); }
		if((x >> 5) == 0x02){ return Vec3(0, (x & 0x1f) - 15, 0); }
		if((x >> 5) == 0x03){ return Vec3(0, 0, (x & 0x1f) - 15); }
		return Vec3();
	}
	static Vec3 decode_far_distance(unsigned int x) noexcept {
		return Vec3(
			((x >>  0) & 0xff) - 30,
			((x >>  8) & 0xff) - 30,
			((x >> 16) & 0xff) - 30);
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


struct Box {
	Vec3 p, q;

	Box() noexcept : p(), q() { }
	Box(const Vec3& p, const Vec3& q) noexcept : p(p), q(q) { normalize(); }

	void normalize() noexcept {
		if(p.x > q.x){ std::swap(p.x, q.x); }
		if(p.y > q.y){ std::swap(p.y, q.y); }
		if(p.z > q.z){ std::swap(p.z, q.z); }
	}

	bool operator==(const Box& b) const noexcept { return p == b.p && q == b.q; }
	bool operator!=(const Box& b) const noexcept { return !(*this == b); }

	bool operator< (const Box& b) const noexcept { return std::make_pair(p, q) <  std::make_pair(b.p, b.q); }
	bool operator<=(const Box& b) const noexcept { return std::make_pair(p, q) <= std::make_pair(b.p, b.q); }
	bool operator> (const Box& b) const noexcept { return std::make_pair(p, q) >  std::make_pair(b.p, b.q); }
	bool operator>=(const Box& b) const noexcept { return std::make_pair(p, q) >= std::make_pair(b.p, b.q); }

	int dim() const noexcept {
		const auto d = p - q;
		return (d.x ? 1 : 0) + (d.y ? 1 : 0) + (d.z ? 1 : 0);
	}

	std::array<Vec3, 8> corners() const noexcept {
		return {
			Vec3(p.x, p.y, p.z), Vec3(p.x, p.y, q.z),
			Vec3(p.x, q.y, p.z), Vec3(p.x, q.y, q.z),
			Vec3(q.x, p.y, p.z), Vec3(q.x, p.y, q.z),
			Vec3(q.x, q.y, p.z), Vec3(q.x, q.y, q.z),
		};
	}

	bool contains(const Vec3& v) const noexcept {
		return
			(p.x <= v.x && v.x <= q.x) &&
			(p.y <= v.y && v.y <= q.y) &&
			(p.z <= v.z && v.z <= q.z);
	}

	bool overlap(const Box& b) const noexcept {
		return
			(q.x >= b.p.x && b.q.x >= p.x) &&
			(q.y >= b.p.y && b.q.y >= p.y) &&
			(q.z >= b.p.z && b.q.z >= p.z);
	}

	template <typename F>
	void each(F&& f) const {
		for(int i = q.z; i <= q.z; ++i){
			for(int j = p.y; j <= q.y; ++j){
				for(int k = p.x; k <= q.x; ++k){ f(i, j, k); }
			}
		}
	}
};

inline std::ostream& operator<<(std::ostream& os, const Box& b){
	return os << "(" << b.p << ", " << b.q << ")";
}

namespace std {
	template <> struct hash<Box> {
		using argument_type = Box;
		using result_type = std::size_t;
		result_type operator()(const argument_type& b) const noexcept {
			result_type h = 0;
			h = h * 1234567891u + std::hash<Vec3>()(b.p);
			h = h * 1234567891u + std::hash<Vec3>()(b.q);
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

	static VoxelGrid load_model(const char *filename){
		std::ifstream ifs(filename, std::ios::in | std::ios::binary);
		ifs.seekg(0, std::ios_base::end);
		const auto tail_pos = ifs.tellg();
		ifs.seekg(0, std::ios_base::beg);
		const size_t size = (tail_pos - ifs.tellg()) * 8;
		size_t r = 1;
		while((r + 1) * (r + 1) * (r + 1) <= size){ ++r; }
		VoxelGrid vg(r);
		for(size_t i = 0; i < size; i += 8){
			int c = ifs.get();
			for(size_t k = i; k < i + 8 && k < r * r * r; ++k){
				const size_t z = k % r, y = (k / r) % r, x = k / (r * r);
				vg(z, y, x) = ((c >> (k - i)) & 1);
			}
		}
		return vg;
	}

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

	int test(const Vec3& a, const Vec3& b) const noexcept {
		const int i_lo = std::min(a.z, b.z), i_hi = std::max(a.z, b.z);
		const int j_lo = std::min(a.y, b.y), j_hi = std::max(a.y, b.y);
		const int k_lo = std::min(a.x, b.x), k_hi = std::max(a.x, b.x);
		int count = 0;
		for(int i = i_lo; i <= i_hi; ++i){
			for(int j = j_lo; j <= j_hi; ++j){
				for(int k = k_lo; k <= k_hi; ++k){
					if((*this)(i, j, k)){ ++count; }
				}
			}
		}
		return count;
	}

	int fill(const Vec3& a, const Vec3& b, uint8_t value) noexcept {
		const int i_lo = std::min(a.z, b.z), i_hi = std::max(a.z, b.z);
		const int j_lo = std::min(a.y, b.y), j_hi = std::max(a.y, b.y);
		const int k_lo = std::min(a.x, b.x), k_hi = std::max(a.x, b.x);
		int count = 0;
		for(int i = i_lo; i <= i_hi; ++i){
			for(int j = j_lo; j <= j_hi; ++j){
				for(int k = k_lo; k <= k_hi; ++k){
					if((*this)(i, j, k) != value){
						(*this)(i, j, k) = value;
						++count;
					}
				}
			}
		}
		return count;
	}

};


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
	FusionS,
	GFill,
	GEmpty
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
	const Vec3& gfill_nd() const noexcept { return v1; }
	const Vec3& gfill_fd() const noexcept { return v2; }
	const Vec3& gempty_nd() const noexcept { return v1; }
	const Vec3& gempty_fd() const noexcept { return v2; }

	Vec3& smove_lld() noexcept { return v1; }
	Vec3& lmove_sld1() noexcept { return v1; }
	Vec3& lmove_sld2() noexcept { return v2; }
	Vec3& fission_nd() noexcept { return v1; }
	Vec3& fill_nd() noexcept { return v1; }
	Vec3& empty_nd() noexcept { return v1; }
	Vec3& fusion_nd() noexcept { return v1; }
	Vec3& gfill_nd() noexcept { return v1; }
	Vec3& gfill_fd() noexcept { return v2; }
	Vec3& gempty_nd() noexcept { return v1; }
	Vec3& gempty_fd() noexcept { return v2; }

	Command& smove_lld(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& lmove_sld1(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& lmove_sld2(const Vec3& v) noexcept { v2 = v; return *this; }
	Command& fission_nd(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& fission_m(int v) noexcept { m = v; return *this; }
	Command& fill_nd(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& empty_nd(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& fusion_nd(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& gfill_nd(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& gfill_fd(const Vec3& v) noexcept { v2 = v; return *this; }
	Command& gempty_nd(const Vec3& v) noexcept { v1 = v; return *this; }
	Command& gempty_fd(const Vec3& v) noexcept { v2 = v; return *this; }

	void write_binary(std::ostream& os) const {
		if(type == CommandType::Halt){
			os.put(0xff);
		}else if(type == CommandType::Wait){
			os.put(0xfe);
		}else if(type == CommandType::Flip){
			os.put(0xfd);
		}else if(type == CommandType::SMove){
			if(smove_lld() == Vec3()){
				os.put(0xfe); // Wait
			}else{
				const auto lld = smove_lld().encode_long_distance();
				os.put(0x04 | ((lld >> 5) << 4));
				os.put(lld & 0x1f);
			}
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
		}else if(type == CommandType::Empty){
			os.put(0x02 | (empty_nd().encode_near_distance() << 3));
		}else if(type == CommandType::GFill){
			const auto fd = gfill_fd().encode_far_distance();
			os.put(0x01 | (gfill_nd().encode_near_distance() << 3));
			os.put((fd >>  0) & 0xff);
			os.put((fd >>  8) & 0xff);
			os.put((fd >> 16) & 0xff);
		}else if(type == CommandType::GEmpty){
			const auto fd = gempty_fd().encode_far_distance();
			os.put(0x00 | (gempty_nd().encode_near_distance() << 3));
			os.put((fd >>  0) & 0xff);
			os.put((fd >>  8) & 0xff);
			os.put((fd >> 16) & 0xff);
		}
	}

	static Command from_binary(std::istream& is){
		const unsigned int f = is.get();
		if(f == 0xff){ return Command(CommandType::Halt); }
		if(f == 0xfe){ return Command(CommandType::Wait); }
		if(f == 0xfd){ return Command(CommandType::Flip); }
		if((f & 0x07) == 0x07){
			return Command(CommandType::FusionP)
				.fusion_nd(Vec3::decode_near_distance(f >> 3));
		}
		if((f & 0x07) == 0x06){
			return Command(CommandType::FusionS)
				.fusion_nd(Vec3::decode_near_distance(f >> 3));
		}
		if((f & 0x07) == 0x03){
			return Command(CommandType::Fill)
				.fill_nd(Vec3::decode_near_distance(f >> 3));
		}
		if((f & 0x07) == 0x02){
			return Command(CommandType::Empty)
				.empty_nd(Vec3::decode_near_distance(f >> 3));
		}
		const unsigned int s = is.get();
		if((f & 0xcf) == 0x04){
			return Command(CommandType::SMove)
				.smove_lld(Vec3::decode_long_distance(((f & 0x30) << 1) | s));
		}
		if((f & 0x0f) == 0x0c){
			return Command(CommandType::LMove)
				.lmove_sld1(Vec3::decode_short_distance((f & 0x30) | (s & 0x0f)))
				.lmove_sld2(Vec3::decode_short_distance(((f & 0xc0) >> 2) | ((s & 0xf0) >> 4)));
		}
		if((f & 0x07) == 0x05){
			return Command(CommandType::Fission)
				.fission_nd(Vec3::decode_near_distance(f >> 3))
				.fission_m(s);
		}
		if((f & 0x0f) == 0x00 || (f & 0x0f) == 0x01){
			const unsigned int t = is.get();
			const unsigned int i = is.get();
			const auto nd = Vec3::decode_near_distance(f >> 3);
			const auto fd = Vec3::decode_far_distance((s << 0) | (t << 8) | (i << 16));
			if((f & 0x0f) == 0x00){
				return Command(CommandType::GEmpty)
					.gempty_nd(nd)
					.gempty_fd(fd);
			}else{
				return Command(CommandType::GFill)
					.gfill_nd(nd)
					.gfill_fd(fd);
			}
		}
		throw std::runtime_error("unknown command");
	}
};

inline std::ostream& operator<<(std::ostream& os, const Command& c){
	os << "Command{ ";
	if(c.type == CommandType::Halt){
		os << "type=Halt";
	}else if(c.type == CommandType::Wait){
		os << "type=Wait";
	}else if(c.type == CommandType::Flip){
		os << "type=Flip";
	}else if(c.type == CommandType::SMove){
		os << "type=SMove, lld=" << c.smove_lld();
	}else if(c.type == CommandType::LMove){
		os << "type=LMove, sld1=" << c.lmove_sld1() << ", sld2=" << c.lmove_sld2();
	}else if(c.type == CommandType::Fission){
		os << "type=Fission, nd=" << c.fission_nd() << ", m=" << c.fission_m();
	}else if(c.type == CommandType::Fill){
		os << "type=Fill, nd=" << c.fill_nd();
	}else if(c.type == CommandType::Empty){
		os << "type=Empty, nd=" << c.empty_nd();
	}else if(c.type == CommandType::FusionP){
		os << "type=FusionP, nd=" << c.fusion_nd();
	}else if(c.type == CommandType::FusionS){
		os << "type=FusionS, nd=" << c.fusion_nd();
	}else if(c.type == CommandType::GFill){
		os << "type=GFill, nd=" << c.gfill_nd() << ", " << c.gfill_fd();
	}else if(c.type == CommandType::GEmpty){
		os << "type=GEmpty, nd=" << c.gempty_nd() << ", " << c.gempty_fd();
	}
	return os << " }";
}


namespace detail {

static inline std::pair<uint64_t, uint64_t> split_seeds(uint64_t s, int m) noexcept {
	for(int i = 0, k = 0; i < 8 * sizeof(s); ++i){
		if(k == m){
			const uint64_t mask = (1ull << i) - 1ull;
			return std::make_pair(s & mask, s & ~mask);
		}
		if(s & (1ull << i)){ ++k; }
	}
	return std::make_pair(s, 0);
}

static inline void format_seeds(std::ostream& os, uint64_t s){
	const int len = 8 * sizeof(s);
	bool is_first = true;
	os << "(";
	for(int head = 0; head < len; ){
		if(!(s & (1ull << head))){
			++head;
			continue;
		}
		int tail = head;
		while(tail < len){
			if(!(s & (1ull << tail))){ break; }
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

}

struct Bot {
	int bid;
	Vec3 pos;
	uint64_t seeds;
};

std::ostream& operator<<(std::ostream& os, const Bot& b){
	os << "Bot{ bid=" << b.bid << ", pos=" << b.pos << ", seeds=";
	detail::format_seeds(os, b.seeds);
	return os << " }";
}


struct CommandError : public std::logic_error {
	Bot bot;
	Command command;

	static std::string make_message(
		const Bot& bot, const Command& command, const std::string& message)
	{
		std::ostringstream oss;
		oss << message << " (bot = " << bot << ", command = " << command << ")";
		return oss.str();
	}

	CommandError(Bot bot, Command command, const std::string& message)
		: std::logic_error(make_message(bot, command, message))
		, bot(bot)
		, command(command)
	{ }
};

static inline bool test_near_distance(const Vec3& v) noexcept {
	if(std::abs(v.x) >= 2 || std::abs(v.y) >= 2 || std::abs(v.z) >= 2){ return false; }
	if(std::abs(v.x) + std::abs(v.y) + std::abs(v.z) > 2){ return false; }
	return true;
}
static inline void throw_test_near_distance(const Vec3& v, const Bot& b, const Command& c){
	if(!test_near_distance(v)){
		throw CommandError(b, c, "invalid near distance");
	}
}

static inline bool test_short_distance(const Vec3& v) noexcept {
	if(v.y == 0 && v.z == 0){ return -5 <= v.x && v.x <= 5; }
	if(v.x == 0 && v.z == 0){ return -5 <= v.y && v.y <= 5; }
	if(v.x == 0 && v.y == 0){ return -5 <= v.z && v.z <= 5; }
	return false;
}
static inline void throw_test_short_distance(const Vec3& v, const Bot& b, const Command& c){
	if(!test_short_distance(v)){
		throw CommandError(b, c, "invalid short linear distance");
	}
}

static inline bool test_long_distance(const Vec3& v) noexcept {
	if(v.y == 0 && v.z == 0){ return -15 <= v.x && v.x <= 15; }
	if(v.x == 0 && v.z == 0){ return -15 <= v.y && v.y <= 15; }
	if(v.x == 0 && v.y == 0){ return -15 <= v.z && v.z <= 15; }
	return false;
}
static inline void throw_test_long_distance(const Vec3& v, const Bot& b, const Command& c){
	if(!test_long_distance(v)){
		throw CommandError(b, c, "invalid long linear distance");
	}
}

static inline bool test_far_distance(const Vec3& v) noexcept {
	if(v.x < -30 || 30 < v.x){ return false; }
	if(v.y < -30 || 30 < v.y){ return false; }
	if(v.z < -30 || 30 < v.z){ return false; }
	return true;
}
static inline void throw_test_far_distance(const Vec3& v, const Bot& b, const Command& c){
	if(!test_far_distance(v)){
		throw CommandError(b, c, "invalid far distance");
	}
}

static inline bool test_position_range(int r, const Vec3& v) noexcept {
	return (0 <= v.x && v.x < r) && (0 <= v.y && v.y < r) && (0 <= v.z && v.z < r);
}
static inline void throw_test_position_range(int r, const Vec3& v, const Bot& b, const Command& c){
	if(!test_position_range(r, v)){
		throw CommandError(b, c, "position out of range");
	}
}


class State {

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
		uint64_t seeds() const noexcept { return m_bot.seeds; }
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
		uint64_t seeds() const noexcept { return m_bot.seeds; }

		void halt(){ m_command = Command(CommandType::Halt); }
		void wait(){ m_command = Command(CommandType::Wait); }
		void flip(){ m_command = Command(CommandType::Flip); }
		void smove(const Vec3& lld){
			const auto c = Command(CommandType::SMove)
				.smove_lld(lld);
			throw_test_long_distance(lld, m_bot, c);
			throw_test_position_range(
				m_state.matrix().size(), m_bot.pos + lld, m_bot, c);
			m_command = c;
		}
		void lmove(const Vec3& sld1, const Vec3& sld2){
			const auto c = Command(CommandType::LMove)
				.lmove_sld1(sld1)
				.lmove_sld2(sld2);
			throw_test_short_distance(sld1, m_bot, c);
			throw_test_short_distance(sld2, m_bot, c);
			throw_test_position_range(
				m_state.matrix().r(), m_bot.pos + sld1 + sld2, m_bot, c);
			m_command = c;
		}
		int fission(const Vec3& nd, int m){
			const auto c = Command(CommandType::Fission)
				.fission_nd(nd)
				.fission_m(m);
			throw_test_near_distance(nd, m_bot, c);
			throw_test_position_range(m_state.matrix().r(), m_bot.pos + nd, m_bot, c);
			const int n = __builtin_popcountll(m_bot.seeds);
			if(m < 0 || n < m + 1){ throw CommandError(m_bot, c, "invalid m"); }
			m_command = c;
			return __builtin_ctzll(m_bot.seeds);
		}
		void fill(const Vec3& nd){
			const auto c = Command(CommandType::Fill)
				.fill_nd(nd);
			throw_test_near_distance(nd, m_bot, c);
			throw_test_position_range(m_state.matrix().r(), m_bot.pos + nd, m_bot, c);
			m_command = c;
		}
		void empty(const Vec3& nd){
			const auto c = Command(CommandType::Empty)
				.empty_nd(nd);
			throw_test_near_distance(nd, m_bot, c);
			throw_test_position_range(m_state.matrix().r(), m_bot.pos + nd, m_bot, c);
			m_command = c;
		}
		void fusion_p(const Vec3& nd){
			const auto c = Command(CommandType::FusionP)
				.fusion_nd(nd);
			throw_test_near_distance(nd, m_bot, c);
			throw_test_position_range(m_state.matrix().r(), m_bot.pos + nd, m_bot, c);
			m_command = c;
		}
		void fusion_s(const Vec3& nd){
			const auto c = Command(CommandType::FusionS)
				.fusion_nd(nd);
			throw_test_near_distance(nd, m_bot, c);
			throw_test_position_range(m_state.matrix().r(), m_bot.pos + nd, m_bot, c);
			m_command = c;
		}
		void gfill(const Vec3& nd, const Vec3& fd){
			const auto c = Command(CommandType::GFill)
				.gfill_nd(nd)
				.gfill_fd(fd);
			throw_test_near_distance(nd, m_bot, c);
			throw_test_far_distance(fd, m_bot, c);
			throw_test_position_range(m_state.matrix().r(), m_bot.pos + nd, m_bot, c);
			throw_test_position_range(m_state.matrix().r(), m_bot.pos + nd + fd, m_bot, c);
			m_command = c;
		}
		void gempty(const Vec3& nd, const Vec3& fd){
			const auto c = Command(CommandType::GEmpty)
				.gempty_nd(nd)
				.gempty_fd(fd);
			throw_test_near_distance(nd, m_bot, c);
			throw_test_far_distance(fd, m_bot, c);
			throw_test_position_range(m_state.matrix().r(), m_bot.pos + nd, m_bot, c);
			throw_test_position_range(m_state.matrix().r(), m_bot.pos + nd + fd, m_bot, c);
			m_command = c;
		}

		void exec(const Command& cmd){
			m_command = cmd;
		}
	};


private:
	struct FusionPositionChecker {
		std::unordered_map<Vec3, Vec3> primaries;
		std::unordered_map<Vec3, Vec3> secondaries;

		void push(const Bot& b, const Command& c){
			if(c.type == CommandType::FusionP){
				primaries.emplace(b.pos, b.pos + c.fusion_nd());
			}else if(c.type == CommandType::FusionS){
				secondaries.emplace(b.pos, b.pos + c.fusion_nd());
			}
		}

		void throw_check(const Bot& b, const Command& c) const {
			if(c.type == CommandType::FusionP){
				const auto it = secondaries.find(b.pos + c.fusion_nd());
				if(it == secondaries.end() || it->second != b.pos){
					throw CommandError(b, c, "unmatched fusion");
				}
			}else if(c.type == CommandType::FusionS){
				const auto it = primaries.find(b.pos + c.fusion_nd());
				if(it == primaries.end() || it->second != b.pos){
					throw CommandError(b, c, "unmatched fusion");
				}
			}
		}
	};

	struct GFillPositionChecker {
		std::unordered_map<Box, uint32_t> fill_flags;
		std::unordered_map<Box, uint32_t> empty_flags;

		void push(const Bot& b, const Command& c){
			if(c.type == CommandType::GFill){
				const auto p0 = b.pos + c.gfill_nd(), p1 = p0 + c.gfill_fd();
				const auto box = Box(p0, p1);
				const auto corners = box.corners();
				uint32_t flags = fill_flags[box];
				for(int c = 0; c < 8; ++c){
					if(!(flags & (1u << c)) && corners[c] == p0){
						flags |= (1u << c);
						break;
					}
				}
				fill_flags[box] = flags;
			}else if(c.type == CommandType::GEmpty){
				const auto p0 = b.pos + c.gfill_nd(), p1 = p0 + c.gfill_fd();
				const auto box = Box(p0, p1);
				const auto corners = box.corners();
				uint32_t flags = empty_flags[box];
				for(int c = 0; c < 8; ++c){
					if(!(flags & (1u << c)) && corners[c] == p0){
						flags |= (1u << c);
						break;
					}
				}
				empty_flags[box] = flags;
			}
		}

		bool test_flags(uint32_t f) const noexcept {
			if(f == 0x11 || f == 0x05 || f == 0x03){ return true; } // segment
			if(f == 0x55 || f == 0x33 || f == 0x0f){ return true; } // plane
			return (f == 0xff); // cube
		}

		void throw_check(const Bot& b, const Command& c) const {
			if(c.type == CommandType::GFill){
				const auto p0 = b.pos + c.gfill_nd(), p1 = p0 + c.gfill_fd();
				const auto it = fill_flags.find(Box(p0, p1));
				if(it == fill_flags.end() || !test_flags(it->second)){
					throw CommandError(b, c, "unmatched gfill");
				}
			}else if(c.type == CommandType::GEmpty){
				const auto p0 = b.pos + c.gfill_nd(), p1 = p0 + c.gfill_fd();
				const auto it = empty_flags.find(Box(p0, p1));
				if(it == empty_flags.end() || !test_flags(it->second)){
					throw CommandError(b, c, "unmatched gempty");
				}
			}
		}
	};

	struct InterferenceChecker {
		std::unordered_map<Box, int> fill_boxes;
		std::unordered_map<Vec3, int> bot_trace;

		void push_path_trace(const Vec3& s, const Vec3& t, int bid){
			Box(s, t).each([this, bid](int z, int y, int x){
				bot_trace.emplace(Vec3(x, y, z), bid);
			});
		}

		void push(const Bot& b, const Command& c){
			if(c.type == CommandType::Wait){
				bot_trace.emplace(b.pos, b.bid);
			}else if(c.type == CommandType::Flip){
				bot_trace.emplace(b.pos, b.bid);
			}else if(c.type == CommandType::SMove){
				push_path_trace(b.pos, b.pos + c.smove_lld(), b.bid);
			}else if(c.type == CommandType::LMove){
				const auto p0 = b.pos, p1 = p0 + c.lmove_sld1(), p2 = p1 + c.lmove_sld2();
				push_path_trace(p0, p1, b.bid);
				push_path_trace(p1, p2, b.bid);
			}else if(c.type == CommandType::Fission){
				bot_trace.emplace(b.pos, b.bid);
				bot_trace.emplace(b.pos + c.fission_nd(), b.bid);
			}else if(c.type == CommandType::Fill){
				const auto p = b.pos + c.fill_nd();
				bot_trace.emplace(b.pos, b.bid);
				fill_boxes[Box(p, p)] = b.bid;
			}else if(c.type == CommandType::Empty){
				const auto p = b.pos + c.fill_nd();
				bot_trace.emplace(b.pos, b.bid);
				fill_boxes[Box(p, p)] = b.bid;
			}else if(c.type == CommandType::FusionP){
				bot_trace.emplace(b.pos, b.bid);
			}else if(c.type == CommandType::FusionS){
				bot_trace.emplace(b.pos, b.bid);
			}else if(c.type == CommandType::GFill){
				const auto fd = c.gfill_fd(), p0 = b.pos + c.gfill_nd(), p1 = p0 + fd;
				bot_trace.emplace(b.pos, b.bid);
				if(fd.x >= 0 && fd.y >= 0 && fd.z >= 0){ fill_boxes[Box(p0, p1)] = b.bid; }
			}else if(c.type == CommandType::GEmpty){
				const auto fd = c.gempty_fd(), p0 = b.pos + c.gempty_nd(), p1 = p0 + fd;
				bot_trace.emplace(b.pos, b.bid);
				if(fd.x >= 0 && fd.y >= 0 && fd.z >= 0){ fill_boxes[Box(p0, p1)] = b.bid; }
			}
		}

		void throw_check_trace(
			const Bot& b, const Command& c, const VoxelGrid& g, const Vec3& p) const
		{
			if(g(p.z, p.y, p.x)){ throw CommandError(b, c, "moving to a filled voxel"); }
			const auto it = bot_trace.find(p);
			if(it == bot_trace.end() || it->second == b.bid){ return; }
			throw CommandError(
				b, c,
				"interference with " + std::to_string(b.bid) +
				" and " + std::to_string(it->second));
		}

		void throw_check_path_trace(
			const Bot& b, const Command& c, const VoxelGrid& g, const Vec3& s, const Vec3& t) const
		{
			Box(s, t).each([this, &b, &c, &g](int z, int y, int x){
				throw_check_trace(b, c, g, Vec3(x, y, z));
			});
		}

		void throw_check_fill_box(const Bot& b, const Command& c, const Box& x) const {
			for(const auto& t : bot_trace){
				if(!x.contains(t.first)){ continue; }
				throw CommandError(
					b, c, "filling/emptying a voxel that occupied by bot " + std::to_string(t.second));
			}
			for(const auto& t : fill_boxes){
				if(t.second == b.bid || !x.overlap(t.first)){ continue; }
				throw CommandError(b, c, "filling/emptying some voxels by multiple bots or groups");
			}
		}

		void throw_check(const Bot& b, const Command& c, const VoxelGrid& g){
			if(c.type == CommandType::Wait){
				throw_check_trace(b, c, g, b.pos);
			}else if(c.type == CommandType::Flip){
				throw_check_trace(b, c, g, b.pos);
			}else if(c.type == CommandType::SMove){
				throw_check_path_trace(b, c, g, b.pos, b.pos + c.smove_lld());
			}else if(c.type == CommandType::LMove){
				const auto p0 = b.pos, p1 = p0 + c.lmove_sld1(), p2 = p1 + c.lmove_sld2();
				throw_check_path_trace(b, c, g, p0, p1);
				throw_check_path_trace(b, c, g, p1, p2);
			}else if(c.type == CommandType::Fission){
				throw_check_trace(b, c, g, b.pos);
				throw_check_trace(b, c, g, b.pos + c.fission_nd());
			}else if(c.type == CommandType::Fill){
				const auto p = b.pos + c.fill_nd();
				throw_check_trace(b, c, g, b.pos);
				throw_check_fill_box(b, c, Box(p, p));
			}else if(c.type == CommandType::Empty){
				const auto p = b.pos + c.fill_nd();
				throw_check_trace(b, c, g, b.pos);
				throw_check_fill_box(b, c, Box(p, p));
			}else if(c.type == CommandType::FusionP){
				throw_check_trace(b, c, g, b.pos);
			}else if(c.type == CommandType::FusionS){
				throw_check_trace(b, c, g, b.pos);
			}else if(c.type == CommandType::GFill){
				const auto fd = c.gfill_fd(), p0 = b.pos + c.gfill_nd(), p1 = p0 + fd;
				throw_check_trace(b, c, g, b.pos);
				if(fd.x >= 0 && fd.y >= 0 && fd.z >= 0){ throw_check_fill_box(b, c, Box(p0, p1)); }
			}else if(c.type == CommandType::GEmpty){
				const auto fd = c.gempty_fd(), p0 = b.pos + c.gempty_nd(), p1 = p0 + fd;
				throw_check_trace(b, c, g, b.pos);
				if(fd.x >= 0 && fd.y >= 0 && fd.z >= 0){ throw_check_fill_box(b, c, Box(p0, p1)); }
			}
		}
	};

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
		, m_bots(1, Bot{ 1, Vec3(0, 0, 0), (((1ull << (max_num_bots - 1)) - 1) << 2) })
		, m_trace()
		, m_pending_commands(1)
		, m_max_num_bots(max_num_bots)
	{ }

	State(const VoxelGrid& initial, int max_num_bots)
		: m_energy(0)
		, m_harmonics(Harmonics::Low)
		, m_matrix(initial)
		, m_bots(1, Bot{ 1, Vec3(0, 0, 0), (((1ull << (max_num_bots - 1)) - 1) << 2) })
		, m_trace()
		, m_pending_commands(1)
		, m_max_num_bots(max_num_bots)
	{ }

	int64_t energy() const noexcept { return m_energy; }
	Harmonics harmonics() const noexcept { return m_harmonics; }

	const VoxelGrid& matrix() const noexcept { return m_matrix; }
	uint8_t  matrix(int i, int j, int k) const noexcept { return m_matrix(i, j, k); }
	uint8_t& matrix(int i, int j, int k)       noexcept { return m_matrix(i, j, k); }

	ConstBotReference bots(size_t i) const noexcept { return ConstBotReference(m_bots[i]); }
	BotReference bots(size_t i) noexcept { return BotReference(*this, m_bots[i], m_pending_commands[i]); }

	size_t bid2index(int bid) const {
		for(size_t i = 0; i < m_bots.size(); ++i){
			if(m_bots[i].bid == bid){ return i; }
		}
		return -1;
	}

	ConstBotReference from_bid(int bid) const {
		const size_t i = bid2index(bid);
		if(i == -1){ throw std::runtime_error("bot not found"); }
		return ConstBotReference(m_bots[i]);
	}
	BotReference from_bid(int bid){
		const size_t i = bid2index(bid);
		if(i == -1){ throw std::runtime_error("bot not found"); }
		return BotReference(*this, m_bots[i], m_pending_commands[i]);
	}

	size_t num_bots() const { return m_bots.size(); }
	size_t max_num_bots() const { return m_max_num_bots; }

	const std::vector<Command>& trace() const noexcept { return m_trace; }

	void commit(){
#ifdef STRONG_VALIDATION
		FusionPositionChecker fusion_checker;
		GFillPositionChecker gfill_checker;
		InterferenceChecker interference_checker;
		for(size_t i = 0; i < m_bots.size(); ++i){
			const auto& b = m_bots[i];
			const auto& c = m_pending_commands[i];
			fusion_checker.push(b, c);
			gfill_checker.push(b, c);
			interference_checker.push(b, c);
		}
		for(size_t i = 0; i < m_bots.size(); ++i){
			const auto& b = m_bots[i];
			const auto& c = m_pending_commands[i];
			fusion_checker.throw_check(b, c);
			gfill_checker.throw_check(b, c);
			interference_checker.throw_check(b, c, m_matrix);
		}
#endif

		// build map to get bot from position for fusion
		std::unordered_map<Vec3, size_t> pos2idx;
		for(size_t i = 0; i < m_bots.size(); ++i){
			const auto& c = m_pending_commands[i];
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
				const auto sp = detail::split_seeds(b.seeds, c.fission_m() + 1);
				new_bots.push_back(Bot{ b.bid, b.pos, sp.second });
				const int new_bid = __builtin_ctzll(sp.first);
				new_bots.push_back(Bot{ new_bid, b.pos + c.fission_nd(), sp.first & ~(1ull << new_bid) });
			}else if(c.type == CommandType::Fill){
				new_bots.push_back(b);
			}else if(c.type == CommandType::Empty){
				new_bots.push_back(b);
			}else if(c.type == CommandType::FusionP){
				const auto it = pos2idx.find(b.pos + c.fusion_nd());
				if(it == pos2idx.end()){ throw CommandError(b, c, "unmatched fusion"); }
				const auto& secondary = m_bots[it->second];
				new_bots.push_back(Bot{ b.bid, b.pos, b.seeds | secondary.seeds | (1ull << secondary.bid) });
			}else if(c.type == CommandType::GFill){
				new_bots.push_back(b);
			}else if(c.type == CommandType::GEmpty){
				new_bots.push_back(b);
			}
		}
		// sort by bid
		std::sort(
			new_bots.begin(), new_bots.end(),
			[](const Bot& a, const Bot& b){ return a.bid < b.bid; });
#ifdef STRONG_VALIDATION
		// validation: check volatility
		Harmonics harmonics = m_harmonics;
		for(size_t i = 0; i < m_bots.size(); ++i){
			const auto& c = m_pending_commands[i];
			if(c.type == CommandType::Flip){ harmonics = flip(harmonics); }
		}
		// validation: grounded or ungrounded
		if(harmonics == Harmonics::Low){
			/*
			const int r = m_matrix.size();
			VoxelGrid grounded(r);
			std::queue<Vec3> q;
			*/
		}
#endif
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
					m_energy -= 12;
				}else{
					m_energy += 3;
				}
			}else if(c.type == CommandType::FusionP){
				m_energy -= 24;
			}else if(c.type == CommandType::GFill){
				const auto fd = c.gfill_fd();
				if(fd.x >= 0 && fd.y >= 0 && fd.z >= 0){
					const int region_size = std::max(fd.x, 1) * std::max(fd.y, 1) * std::max(fd.z, 1);
					const int fill_count = m_matrix.fill(
						b.pos + c.gfill_nd(), b.pos + c.gfill_nd() + c.gfill_fd(), 1);
					m_energy += 12 * fill_count;
					m_energy +=  6 * (region_size - fill_count);
				}
			}else if(c.type == CommandType::GEmpty){
				const auto fd = c.gempty_fd();
				if(fd.x >= 0 && fd.y >= 0 && fd.z >= 0){
					const int region_size = std::max(fd.x, 1) * std::max(fd.y, 1) * std::max(fd.z, 1);
					const int fill_count = m_matrix.fill(
						b.pos + c.gempty_nd(), b.pos + c.gempty_nd() + c.gempty_fd(), 0);
					m_energy -= 12 * fill_count;
					m_energy +=  3 * (region_size - fill_count);
				}
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

	void dump_pending_commands(std::ostream& os) const {
		for(size_t i = 0; i < m_bots.size(); ++i){
			os << m_bots[i] << ": " << m_pending_commands[i] << std::endl;
		}
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

void dump(const State& s){
	for(size_t i = 0; i < s.num_bots(); ++i){
		std::cout << i << ": " << s.bots(i) << std::endl;
	}
	std::cout << std::endl;
}

#endif
