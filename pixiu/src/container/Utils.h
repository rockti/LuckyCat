#pragma once

#include <vector>

template<class T>
bool contains(std::vector<T> const& c, T const& v) {
	return std::find(c.begin(), c.end(), v) != c.end();
}
