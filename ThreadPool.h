#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool {
public:
    ThreadPool(size_t);
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task queue
    std::queue< std::function<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};
 
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            // lambda表达式，无限循环执行任务队列中的task
            [this]
            {
                for(;;)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        // 尝试获取锁
                        // 能够获取锁，判断条件是否成立，成立占有锁，向下执行执行完后，释放锁；条件不成立，释放锁，睡眠，需要通过notifyone或all唤醒
                        // 不能获取锁，阻塞，尝试获取锁
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); }); // this->stop可以保证析构notifyall时该线程可以占有锁，保证线程正常结束
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    // 取task的时候线程保护，执行task的时候没有保护，task需要线程安全，依赖task的实现
                    task();
                }
            }
        );
}

// add new work item to the pool
// 函数名F，参数列表Args
// std::result_of<F(Args...)>::type推导函数F的返回值类型作为std::future的模板参数
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    // 推导函数F的返回值类型，起别名return_type
    using return_type = typename std::result_of<F(Args...)>::type;

    // 创建了一个共享指针，类型是std::packaged_task<return_type()>
    // 封装函数和参数
    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        // 将任务的lambda表达式加入队列
        tasks.emplace([task](){ (*task)(); });
    }
    condition.notify_one(); // 通知一个线程可以执行任务
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all(); // 通知所有线程，结束线程
    for(std::thread &worker: workers) // 阻塞主线程，等待所有线程结束
        worker.join();
}

#endif
