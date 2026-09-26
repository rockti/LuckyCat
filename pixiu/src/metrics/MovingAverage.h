#pragma once

#include "type/Money.h"

#include <vector>

template<class T>
class SimpleMovingAverage {
public:
	SimpleMovingAverage(int size) :m_array(size),m_capacity(size) {

	}
	int Size() const {
		if (m_count < m_capacity) {
			return m_count;
		}
		return m_capacity;
	}

	int Capacity() const {
		return m_capacity;
	}

	bool IsFull() const {
		return m_count >= m_capacity;
	}

	void Put(T const& v) {
		int last_index = (m_count++) % m_capacity;
		T first_value = 0;
		int size = m_count;
		if (m_count > m_capacity) {
			first_value = m_array[last_index];
			size = m_capacity;
		}
		m_array[last_index] = v;
		m_total += v - first_value;
		m_mean = m_total / size;
		
	}

	money Average() const {
		return m_mean;
	}
private:
	std::vector<T> m_array;
	money m_total = 0;
	money m_mean = 0;
	int m_count = 0;
	int m_capacity = 0;
};

enum class Crossover {
	NA,
	Golden,
	Death,
};

class AverageCrossover {
public:
	AverageCrossover(int fastSize, int slowSize):m_fast(fastSize),m_slow(slowSize) {
		assert(slowSize > fastSize);

	}
	Crossover Put(money price) {
		m_fast.Put(price);
		m_slow.Put(price);
		m_crossover = crossover();
		return m_crossover;
	}

	Crossover GetCrossover() const {
		return m_crossover;
	}
private:

	Crossover crossover() {
		if (!m_slow.IsFull()) {
			return Crossover::NA;
		}
		if (m_fast.Average() > m_slow.Average()) {
			return Crossover::Golden;
		} else {
			return Crossover::Death;
		}
	}

	SimpleMovingAverage<money> m_fast;
	SimpleMovingAverage<money> m_slow;
	Crossover m_crossover = Crossover::NA;
};
