#pragma once

#include <atomic>
#include <thread>


namespace atomic {

inline void thread_fence()
{
    std::atomic_thread_fence(std::memory_order_acq_rel);
}

inline void fence()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

inline void acquire()
{
    std::atomic_thread_fence(std::memory_order_acquire);
}

inline void release()
{
    std::atomic_thread_fence(std::memory_order_release);
}

inline void cpu_pause()
{
    std::this_thread::yield();
}
}
