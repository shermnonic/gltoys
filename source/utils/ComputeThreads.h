#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>

class ComputeThreads
{
public:
    //typedef void (*ComputeFunc)( int );
    typedef std::function<void(int)> ComputeFunc;

    ComputeThreads( int numThreads, ComputeFunc computeFun )
        : m_numThreads(numThreads),
          m_threadStates(numThreads),
          m_threads(numThreads)
    {
        for(int i=0; i < m_numThreads; ++i)
        {
            auto fun = [this, computeFun]( int slice )
            {
                while( !m_threadStates[slice].kill.load() )
                {
                    // yield until dirty
                    while( !m_threadStates[slice].dirty.load() )
                    {
                        if( m_threadStates[slice].kill.load() )
                        {
                            // thread was killed, return to terminate
                            return;
                        }
                        else
                        {
                            std::this_thread::yield();
                        }
                    }

                    // do actual computation and time it
                    auto const startTime = std::chrono::high_resolution_clock::now();
                    computeFun(slice);
                    auto const endTime = std::chrono::high_resolution_clock::now();
                    uint64_t const duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

                    // update state
                    auto& state = m_threadStates[slice];
                    state.count++;
                    state.dirty.store(false);
                    state.lastComputeDuration.store(duration);
                }
            };

            m_threads.at(i) =  std::thread(fun, i);
        }
    }

    ~ComputeThreads()
    {
        for(int i=0; i < m_numThreads; ++i)
        {
            m_threadStates[i].kill.store(true);
            m_threads.at(i).join();
        }
    }

    bool ready() const
    {
        return numDirty()==0;
    }

    int numDirty() const
    {
        int count=0;
        for(int i=0; i < m_numThreads; ++i)
            if(m_threadStates[i].dirty.load())
                count++;
        return count;
    }

    void launchAll()
    {
        for(int i=0; i < m_numThreads; ++i)
            m_threadStates[i].dirty.store(true);
    }

    uint64_t getTotalComputationTime() const
    {
        uint64_t total = 0;
        for(auto const& state : m_threadStates)
        {
            total += state.lastComputeDuration.load();
        }
        return total;
    }


private:
    struct ThreadState
    {
        std::atomic<bool> dirty = false;
        std::atomic<bool> kill = false;
        std::atomic<unsigned> count = 0;
        std::atomic<uint64_t> lastComputeDuration = 0;
    };

    int m_numThreads;
    std::vector<ThreadState> m_threadStates;
    std::vector<std::thread> m_threads;
};
