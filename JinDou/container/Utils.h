#pragma once

#include <vector>

template<class T>
bool contains(std::vector<T> const& c, T const& v) {
	return std::find(c.begin(), c.end(), v) != c.end();
}

template <typename Map, typename F>
void map_erase_if(Map& m, F pred)
{
	for (typename Map::iterator i = m.begin();
		(i = std::find_if(i, m.end(), pred)) != m.end();
		m.erase(i++));
}

template <typename Map, typename F, typename A>
void map_erase_if(Map& m, F pred, A action)
{
	for (typename Map::iterator i = m.begin();
		(i = std::find_if(i, m.end(), pred)) != m.end();) {
		action(i->second);
		m.erase(i++);
	}
}