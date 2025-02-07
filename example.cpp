#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.h"

int GetMaxValue(const std::vector<int>& nums, int start, int end)
{
    int ret = INT_MIN;
    for (int i = start; i <= end; ++i)
    {
        if (nums[i] > ret)
        {
            ret = nums[i];
        }
    }
    return ret;
}

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

    // 例子，随机生成n个数，找出最大值
    int n = 10000000;
    std::vector<int> nums;
    std::srand(static_cast<unsigned int>(std::time(0)));
    for (int i = 0; i < n; ++i)
    {
        nums.emplace_back(std::rand() % n + 1);
    }

    // 单线程 直接比较
    {
        auto start = std::chrono::high_resolution_clock::now();
        int ret = INT_MIN;
        for (int i = 0; i < n; ++i)
        {
            if (nums[i] > ret)
            {
                ret = nums[i];
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_ms = end - start;
        std::cout << ret << "  " << elapsed_ms.count() << std::endl;
    }


    // 多线程，先分成m组，分别比较出最大值，然后再比较m个数的最大值
    {
        auto start = std::chrono::high_resolution_clock::now();
        int m = 10; // 此处合理分组，过大会导致enqueue构造任务花费大量的时间！
        int count = n / m;
        ThreadPool maxPool(5);
        std::vector<std::future<int>> maxValues;
        for (int i = 0; i < m; ++i)
        {
            maxValues.emplace_back(maxPool.enqueue(GetMaxValue, nums, i * count, (i + 1) * count - 1));
        }

        auto mid = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> midelapsed = mid - start;
        std::cout << midelapsed.count() << std::endl;

        int ret = INT_MIN;
        for (auto&& maxValue : maxValues)
        {
            int threadret = maxValue.get();
            if (threadret > ret)
            {
                ret = threadret;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_ms = end - start;
        std::cout << ret << "  " << elapsed_ms.count() << std::endl;
    }
    
    return 0;
}
