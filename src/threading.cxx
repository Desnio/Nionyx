#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

class Threading
{
public:
    Threading(int threads)
    {
        running = true;

        for (int i = 0; i < threads; i++)
        {
            workers.emplace_back(&Threading::Thread, this);
        }
    }

    ~Threading()
    {
        running = false;

        for (auto& t : workers)
        {
            if (t.joinable())
                t.join();
        }
    }

    void Enque(std::function<void()> func)
    {
        std::lock_guard<std::mutex> lock(mtx);
        funcs.push_back(func);
    }

private:
    void Thread()
    {
        while (running)
        {
            std::function<void()> task;

            {
                std::lock_guard<std::mutex> lock(mtx);

                if (!funcs.empty())
                {
                    task = funcs.back();
                    funcs.pop_back();
                }
            }

            if (task)
            {
                task();
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    }

private:
    std::vector<std::thread> workers;
    std::vector<std::function<void()>> funcs;

    std::mutex mtx;
    std::atomic<bool> running{false};
};