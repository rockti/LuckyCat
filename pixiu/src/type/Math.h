#pragma once

#include <algorithm>
namespace DoubleComparer {
	typedef bool (*CompareFunction)(double l, double r, double epsilon);

	inline bool equal(double l, double r, double epsilon = 0.0001) {
		return fabs(l - r) < epsilon;
	}

	inline bool less(double l, double r, double epsilon = 0.0001) {
		double difference = r - l;
		return difference > epsilon;
	}

	inline bool greater(double l, double r, double epsilon = 0.0001) {
		double difference = l - r;
		return difference > epsilon;
	}

	inline bool less_equal(double l, double r, double epsilon = 0.0001) {
		return less(l, r, epsilon) || equal(l, r, epsilon);
	}

	inline bool greater_equal(double l, double r, double epsilon = 0.0001) {
		return greater(l, r, epsilon) || equal(l, r, epsilon);
	}

	inline double GetLess(double l, double r, double espilon = 0.0001) {
		if (less(l, r, espilon)) {
			return l;
		}
		return r;
	}

	inline double GetGreater(double l, double r, double espilon = 0.0001) {
		if (greater(l, r, espilon)) {
			return l;
		}
		return r;
	}
}
