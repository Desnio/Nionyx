#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

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
        {
            std::lock_guard<std::mutex> lock(mtx);
            running = false;
        }

        cv.notify_all(); // wake all threads so they can exit

        for (auto& t : workers)
        {
            if (t.joinable())
                t.join();
        }
    }

    void Enque(std::function<void()> func)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            funcs.push_back(func);
        }

        cv.notify_one(); // wake ONE worker thread
    }

private:
    void Thread()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mtx);

    
                cv.wait(lock, [this]()
                {
                    return !funcs.empty() || !running;
                });

                if (!running && funcs.empty())
                    return;

                task = funcs.back();
                funcs.pop_back();
            }

            task(); // run outside lock
        }
    }

    std::vector<std::thread> workers;
    std::vector<std::function<void()>> funcs;

    std::mutex mtx;
    std::condition_variable cv;
    bool running = false;
};