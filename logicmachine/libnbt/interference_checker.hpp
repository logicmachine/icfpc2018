#ifndef LIBNBT_INTERFERENCE_CHECKER_HPP
#define LIBNBT_INTERFERENCE_CHECKER_HPP

#include <unordered_set>
#include "../../nbtasm/nbtasm.hpp"

class InterferenceChecker {
private:
	std::unordered_set<Vec3> m_positions;
public:
	InterferenceChecker() : m_positions() { }
	InterferenceChecker(const State& s)
		: m_positions()
	{
		for(int i = 0; i < s.num_bots(); ++i){ m_positions.insert(s.bots(i).pos()); }
	}
	bool test(const Vec3& p) const { return m_positions.find(p) == m_positions.end(); }
	bool set(const Vec3& p){ return m_positions.insert(p).second; }
};

#endif
