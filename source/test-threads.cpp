#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include "utils/ComputeThreads.h"

std::mutex g_mutex_cout;

void compute( int id )
{
    { std::lock_guard<std::mutex> guard(g_mutex_cout); std::cout << "start " << id << std::endl; }
    std::this_thread::sleep_for(std::chrono::seconds(id+1));
    { std::lock_guard<std::mutex> guard(g_mutex_cout); std::cout << "end " << id << std::endl; }
}

int main()
{
    ComputeThreads pool(4,compute);
    pool.launchAll();
    while(pool.numDirty() > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        { std::lock_guard<std::mutex> guard(g_mutex_cout); std::cout << "waiting " << pool.numDirty() << " dirty" << std::endl; }
    }
    std::cout << "compute finished" << std::endl;
}
