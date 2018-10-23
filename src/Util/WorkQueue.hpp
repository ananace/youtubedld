#pragma once

#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace Util
{

class WorkQueue
{
public:
    WorkQueue(uint8_t aCount = 0);
    WorkQueue(const WorkQueue&) = delete;
    WorkQueue(WorkQueue&&) = default;
    ~WorkQueue();

    void setWorkerCount(uint8_t aCount);
    uint8_t getWorkerCount() const;

    void start();
    void stop();

    template<typename Ret, typename Func>
    std::future<Ret> queueTask(Func&& aFunc);
    template<typename Ret, typename Func, typename... Args>
    std::future<Ret> queueTask(Func&& aFunc, Args&&... aArgs);

private:
    void _workThread();

    std::condition_variable m_queueCV;
    std::mutex m_queueMutex;
    std::vector<std::function<void()>> m_taskQueue;
    std::vector<std::thread> m_workThreads;

    bool m_running;
};

}

template<typename Ret, typename Func>
std::future<Ret> Util::WorkQueue::queueTask(Func&& aFunc)
{
    auto task = std::make_shared<std::packaged_task<Ret()>>(std::forward<Func>(aFunc));
    auto future = task->get_future();

    {
        std::lock_guard<std::mutex> _lock(m_queueMutex);
        m_taskQueue.push_back([task = std::move(task)]() { (*task)(); } );
    }

    m_queueCV.notify_one();

    return future;
}
template<typename Ret, typename Func, typename... Args>
std::future<Ret> Util::WorkQueue::queueTask(Func&& aFunc, Args&&... aArgs)
{
    auto task = std::make_shared<std::packaged_task<Ret()>>(std::bind(std::forward<Func>(aFunc), std::forward<Args>(aArgs)...));
    auto future = task->get_future();

    {
        std::lock_guard<std::mutex> _lock(m_queueMutex);
        m_taskQueue.push_back([task = std::move(task)]() { (*task)(); });
    }

    m_queueCV.notify_one();

    return future;
}
