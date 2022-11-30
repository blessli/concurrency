#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
std::string FormatTimeNow(const char*);
class Semaphore {
public:
  explicit Semaphore(int count = 0) : count_(count) {
  }

  void Signal() {
    std::unique_lock<std::mutex> lock(mutex_);
    ++count_;
    cv_.notify_one();
  }

  void Wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [=] { return count_ > 0; });
    --count_;
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
};

std::mutex g_io_mutex;
Semaphore g_semaphore(3);

void Worker() {
  g_semaphore.Wait();

  std::thread::id thread_id = std::this_thread::get_id();

  std::string now = FormatTimeNow("%H:%M:%S");
  {
    std::lock_guard<std::mutex> lock(g_io_mutex);
    std::cout << "Thread " << thread_id << ": wait succeeded" << " (" << now << ")" << std::endl;
  }

  // Sleep 1 second to simulate data processing.
  std::this_thread::sleep_for(std::chrono::seconds(1));

  g_semaphore.Signal();
}

int main() {
  const std::size_t SIZE = 3;

  std::vector<std::thread> v;
  v.reserve(SIZE);

  for (std::size_t i = 0; i < SIZE; ++i) {
    v.emplace_back(&Worker);
  }

  for (std::thread& t : v) {
    t.join();
  }
  
  return 0;
}
std::string FormatTimeNow(const char* format) {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm* now_tm = std::localtime(&now_c);

  char buf[20];
  std::strftime(buf, sizeof(buf), format, now_tm);
  return std::string(buf);
}