#pragma once

#include "atomic/Atomic64.h"
#include <cstdint>
#include <chrono>
#include <thread>
#include <algorithm>
#ifdef min
#undef min
#endif
namespace atomic {

	/*
	while (isRunning) {
		if (!hasWork()) { 
			idleStrategy.reset();     
			while (!hasWork()) { 
				if (!isRunning) { return; }       
				idleStrategy.idle();
			} 
		}   
		doWork();
	}
	*/

	class BusySpinIdleStrategy {
	public:
		BusySpinIdleStrategy() = default;

		inline void idle(int workCount) {
			if (workCount > 0) {
				return;
			}
			pause();
		}

		inline void reset() {
		}

		inline void idle() {
			pause();
		}

		inline static void pause() {
			atomic::cpu_pause();
		}

	private:
	};



	/*
	while (isRunning) {     
		idleStrategy.idle(doWork()); 
	}


	*/
	/*
	while (isRunning) {
		if (!hasWork()) { 
			idleStrategy.reset();     
			while (!hasWork()) { 
				if (!isRunning) { return; }       
				idleStrategy.idle(); 
			} 
		}  
		doWork();
	}
	*/
	static const size_t CACHE_LINE_LENGTH = 64;

	static const std::uint8_t BACKOFF_STATE_NOT_IDLE = 0;
	static const std::uint8_t BACKOFF_STATE_SPINNING = 1;
	static const std::uint8_t BACKOFF_STATE_YIELDING = 2;
	static const std::uint8_t BACKOFF_STATE_PARKING = 3;

	class BackoffIdleStrategy {
	public:
		explicit BackoffIdleStrategy(
			std::int64_t maxSpins = 10,
			std::int64_t maxYields = 20,
			std::chrono::duration<long, std::nano> minParkPeriodNs = std::chrono::duration<long, std::nano>(1000),
			std::chrono::duration<long, std::nano> maxParkPeriodNs = std::chrono::duration<long, std::milli>(1))
			:
			m_prePad(),
			m_maxSpins(maxSpins),
			m_maxYields(maxYields),
			m_minParkPeroidNs(minParkPeriodNs),
			m_maxParkPeriodNs(maxParkPeriodNs),
			m_spins(0),
			m_yields(0),
			m_parkPeriodNs(minParkPeriodNs),
			m_state(BACKOFF_STATE_NOT_IDLE),
			m_postPad() {
		}

		inline void idle(int workCount) {
			if (workCount > 0) {
				reset();
			} else {
				idle();
			}
		}

		inline void reset() {
			m_spins = 0;
			m_yields = 0;
			m_parkPeriodNs = m_minParkPeroidNs;
			m_state = BACKOFF_STATE_NOT_IDLE;
		}

		inline void idle() {
			switch (m_state) {
			case BACKOFF_STATE_NOT_IDLE:
				m_state = BACKOFF_STATE_SPINNING;
				m_spins++;
				break;

			case BACKOFF_STATE_SPINNING:
				atomic::cpu_pause();
				if (++m_spins > m_maxSpins) {
					m_state = BACKOFF_STATE_YIELDING;
					m_yields = 0;
				}
				break;

			case BACKOFF_STATE_YIELDING:
				if (++m_yields > m_maxYields) {
					m_state = BACKOFF_STATE_PARKING;
					m_parkPeriodNs = m_minParkPeroidNs;
				} else {
					std::this_thread::yield();
				}
				break;

			case BACKOFF_STATE_PARKING:
			default:
				std::this_thread::sleep_for(m_parkPeriodNs);
				m_parkPeriodNs = std::min(m_parkPeriodNs * 2, m_maxParkPeriodNs);
				break;
			}
		}

	protected:
		std::uint8_t m_prePad[CACHE_LINE_LENGTH * 2];
		const std::int64_t m_maxSpins;
		const std::int64_t m_maxYields;
		const std::chrono::duration<long, std::nano> m_minParkPeroidNs;
		const std::chrono::duration<long, std::nano> m_maxParkPeriodNs;
		std::int64_t m_spins;
		std::int64_t m_yields;
		std::chrono::duration<long, std::nano> m_parkPeriodNs;
		std::uint8_t m_state;
		std::uint8_t m_postPad[CACHE_LINE_LENGTH * 2];
	};

	class NoOpIdleStrategy {
	public:
		NoOpIdleStrategy() = default;

		inline void idle(int workCount) {
		}

		inline void reset() {
		}

		inline void idle() {
		}

	private:
	};

	class SleepingIdleStrategy {
	public:
		explicit SleepingIdleStrategy(const std::chrono::duration<long, std::milli> duration) :
			m_duration(duration) {
		}

		inline void idle(int workCount) {
			if (0 == workCount) {
				std::this_thread::sleep_for(m_duration);
			}
		}

		inline void reset() {
		}

		inline void idle() {
			std::this_thread::sleep_for(m_duration);
		}

	private:
		std::chrono::duration<long, std::milli> m_duration;
	};


	class YieldingIdleStrategy {
	public:
		YieldingIdleStrategy() = default;

		inline void idle(int workCount) {
			if (workCount > 0) {
				return;
			}

			std::this_thread::yield();
		}

		inline void reset() {
		}

		inline void idle() {
			std::this_thread::yield();
		}

		inline static void pause() {
			atomic::cpu_pause();
		}


	private:
	};

}
