#include "threading.hxx"

std::vector<std::function<void()>> funcs;

bool running;

Threading::Thread()
{

while(running)
{

if(funcs.size() > 0;)
{
funcs[1]();
}
else
{std::this_thread::sleep_for(std::chrono::milliseconds(20));}

}

}

Threading::Enque(std::function<void()> func)
{
funcs.push_back(func);
}

Threading::Threading(int threads)
{
std::vector<std::thread> workers;

for(int i = 0; i < threads; i++)
{
std::thread thread(Thread);
workers.push_back(thread)
}

}