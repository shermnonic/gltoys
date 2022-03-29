#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <functional>

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
            auto fun = [this,computeFun]( int slice )
            {
                while( !m_threadStates[slice].kill.load() )
                {
                    // yield until dirty
                    while( !m_threadStates[slice].dirty.load() )
                        if( m_threadStates[slice].kill.load() )
                            return;
                        else
                            std::this_thread::yield();

                    // do actual computation
                    computeFun(slice);

                    m_threadStates[slice].count++;
                    m_threadStates[slice].dirty.store(false);
                }
            };

            m_threads.at(i) =  std::thread(fun,i);
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

private:
    struct ThreadState
    {
        std::atomic<bool> dirty = false;
        std::atomic<bool> kill = false;
        std::atomic<unsigned> count = 0;
    };

    int m_numThreads;
    std::vector<ThreadState> m_threadStates;
    std::vector<std::thread> m_threads;
};
