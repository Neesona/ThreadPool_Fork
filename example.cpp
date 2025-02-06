#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.h"

int main()
{
    
    ThreadPool pool(4);
    std::vector< std::future<int> > results;

    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }

    // get方法会阻塞在主线程上，直到有可用的结果
    // 如果不调用get方法，需要在ThreadPool添加一个阻塞方法，参考ThreadPool的析构函数
    for(auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    
    return 0;
}
