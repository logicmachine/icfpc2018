#ifndef LIBNBT_PLANER_ROUTER_HPP
#define LIBNBT_PLANER_ROUTER_HPP

#include <unordered_set>
#include <queue>
#include <cstdint>
#include "vec2.hpp"
#include "array2d.hpp"

static std::vector<Vec2> planer_routing(
	Array2D<uint8_t>& obstacles,
	std::vector<Vec2> sources,
	std::vector<Vec2> targets)
{
	const Vec2 area(obstacles.cols(), obstacles.rows());

	std::unordered_set<Vec2> targets_set(targets.begin(), targets.end());
	std::vector<bool> source_done(sources.size(), false);

	std::vector<Vec2> result(sources.size());
	while(!targets_set.empty()){
		Array2D<int> src_indices(area.y, area.x, -1);
		Array2D<Vec2> prev(area.y, area.x);
		std::queue<Vec2> q;
	
		for(int i = 0; i < sources.size(); ++i){
			if(source_done[i]){ continue; }
			src_indices(sources[i]) = i;
			prev(sources[i]) = sources[i];
			q.push(sources[i]);
		}
		if(q.empty()){ break; }

		while(!q.empty()){
			const auto& cur = q.front();
			if(targets_set.find(cur) != targets_set.end()){ break; }
			q.pop();
			for(int i = 1; i <= MAX_LONG_DISTANCE; ++i){
				const Vec2 next = cur + Vec2(i, 0);
				if(!next.region_check(area) || obstacles(next)){ break; }
				if(src_indices(next) >= 0){ continue; }
				src_indices(next) = src_indices(cur);
				prev(next) = cur;
				q.push(next);
			}
			for(int i = 1; i <= MAX_LONG_DISTANCE; ++i){
				const Vec2 next = cur + Vec2(0, i);
				if(!next.region_check(area) || obstacles(next)){ break; }
				if(src_indices(next) >= 0){ continue; }
				src_indices(next) = src_indices(cur);
				prev(next) = cur;
				q.push(next);
			}
			for(int i = -1; i >= -MAX_LONG_DISTANCE; --i){
				const Vec2 next = cur + Vec2(i, 0);
				if(!next.region_check(area) || obstacles(next)){ break; }
				if(src_indices(next) >= 0){ continue; }
				src_indices(next) = src_indices(cur);
				prev(next) = cur;
				q.push(next);
			}
			for(int i = -1; i >= -MAX_LONG_DISTANCE; --i){
				const Vec2 next = cur + Vec2(0, i);
				if(!next.region_check(area) || obstacles(next)){ break; }
				if(src_indices(next) >= 0){ continue; }
				src_indices(next) = src_indices(cur);
				prev(next) = cur;
				q.push(next);
			}
		}
		if(q.empty()){ break; }

		Vec2 cur(q.front());
		targets_set.erase(cur);
		const int sid = src_indices(cur);
		source_done[sid] = true;
		while(prev(cur) != cur){
			const auto t = prev(cur);
			result[sid] = cur - t;
			cur = t;
		}

		const auto d = result[sid];
		if(d.x < 0){
			for(int i = 0; i >= d.x; --i){ obstacles(cur + Vec2(i, 0)) = 1; }
		}else if(d.x > 0){
			for(int i = 0; i <= d.x; ++i){ obstacles(cur + Vec2(i, 0)) = 1; }
		}else if(d.y < 0){
			for(int i = 0; i >= d.y; --i){ obstacles(cur + Vec2(0, i)) = 1; }
		}else if(d.y > 0){
			for(int i = 0; i <= d.y; ++i){ obstacles(cur + Vec2(0, i)) = 1; }
		}
	}
	return result;
}

#endif
