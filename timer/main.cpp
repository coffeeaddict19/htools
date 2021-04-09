#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>
#include <memory>

namespace
{
  volatile std::sig_atomic_t gSignalStatus;
}

void signal_handler(int signal)
{
  gSignalStatus = signal;
}

int main(){
  std::signal(SIGINT, signal_handler);
  const char* kHelloMessage = "Hello and Welcome to the Timer";
  const char* kGoodbyeMessage = "Have a Nice Day Sir";
  std::cout << kHelloMessage << std::endl;
  int* secondsToSleepFor = new int();
  *secondsToSleepFor = 1;
  std::unique_ptr<unsigned long long> numberOfIterations = std::make_unique<unsigned long long>(0);
  std::cout << "start [Send SIGINT to Terminate]" <<std::endl;
  while(gSignalStatus != SIGINT){
    std::time_t result = std::time(nullptr);
    std::cout << std::ctime(&result);
    std::this_thread::sleep_for (std::chrono::seconds(*secondsToSleepFor));
    *numberOfIterations.get() +=1;
  }
  if(secondsToSleepFor != nullptr) delete secondsToSleepFor;
  std::cout << "end [Iterations: " << *numberOfIterations.get() << "]" << std::endl;
  std::cout << kGoodbyeMessage << std::endl;

  return 0;
}
