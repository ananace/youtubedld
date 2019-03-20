#include "WorkQueue.hpp"

using Util::WorkQueue;

WorkQueue::WorkQueue(uint8_t aCount)
    : m_running(false)
{
    if (aCount == 0)
        aCount = 4; // TODO: detect

    m_workThreads.resize(aCount);
}

WorkQueue::~WorkQueue()
{
    stop();
}

void WorkQueue::setWorkerCount(uint8_t aCount)
{
    auto running = m_running;

    if (running)
        stop();

    m_workThreads.resize(aCount);

    if (running)
        start();
}
uint8_t WorkQueue::getWorkerCount() const
{
    return uint8_t(m_workThreads.size());
}

void WorkQueue::start()
{
    m_running = true;
    for (auto& thr : m_workThreads)
        thr = std::thread(&WorkQueue::_workThread, this);
}

void WorkQueue::stop()
{
    m_running = false;
    m_queueCV.notify_all();

    for (auto& thread : m_workThreads)
        if (thread.joinable())
            thread.join();
}

void WorkQueue::_workThread()
{
    std::unique_lock<std::mutex> _lock(m_queueMutex, std::defer_lock);
    while (m_running)
    {
        _lock.lock();
        m_queueCV.wait(_lock, [this]() { return !m_taskQueue.empty(); });

        if (!m_taskQueue.empty())
        {
            auto job = m_taskQueue.front();
            m_taskQueue.erase(m_taskQueue.begin());
            _lock.unlock();

            job();
        }
        else
            _lock.unlock();
    }
}
