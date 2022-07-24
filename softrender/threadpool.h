#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <future>
#include <condition_variable>

class FixedThreadPool 
{
public:
    FixedThreadPool(size_t thread_count)
        : _data(std::make_shared<Data>()) 
    {
        _data->thread_count = thread_count;
        for (size_t i = 0; i < thread_count; ++i) 
        {
            std::thread([data = _data] {
                std::unique_lock<std::mutex> lk(data->mtx);
                while(true)
                {
                    if (!data->tasks.empty()) 
                    {
                        auto [task, prom] = std::move(data->tasks.front());
                        data->tasks.pop();
                        lk.unlock();
                        task();
                        prom.set_value();
                        lk.lock();
                    }
                    else if (data->is_shutdown_) 
                    {
                        break;
                    }
                    else 
                    {
                        data->cond.wait(lk);
                    }
                }
            }).detach();
        }
    }

    FixedThreadPool() = default;
	
    FixedThreadPool(FixedThreadPool&&) = default;

    ~FixedThreadPool() 
    {
        if (_data) 
        {
            {
                std::lock_guard<std::mutex> lk(_data->mtx);
                _data->is_shutdown_ = true;
            }
            _data->cond.notify_all();
        }
    }

    template <class F>
    std::future<void> execute(F&& task) 
    {
        std::future<void> fut;
        {
            std::lock_guard<std::mutex> lk(_data->mtx);
            std::promise<void> p;
			fut = p.get_future();
            _data->tasks.emplace(std::forward<F>(task), std::move(p));
        }
        _data->cond.notify_one();
        return fut;
    }

    size_t thread_count() const
    {
        return _data->thread_count;
    }

private:

    struct Data 
    {
        size_t thread_count;
        std::mutex mtx;
        std::condition_variable cond;
        bool is_shutdown_ = false;
        std::queue<
            std::pair<std::function<void()>, std::promise<void>>,
            std::list<std::pair<std::function<void()>, std::promise<void>>>
        > tasks;
    };
	
    std::shared_ptr<Data> _data;
};

#endif 