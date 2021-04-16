#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>
#include <memory>
#include <inttypes.h>

namespace
{
  volatile std::sig_atomic_t gSignalStatus;
}

void signal_handler(int signal)
{
  gSignalStatus = signal;
}

void print_address(const char* name, const void* ptr_to_print_address_of);

int main(){
  std::signal(SIGINT, signal_handler);

  const char* kHelloMessage = "Hello and Welcome to 'timer'";
  print_address("kHelloMessage", static_cast<const void*>(kHelloMessage));

  const char* kGoodbyeMessage = "Have a Nice Day Sir";
  print_address("kGoodbyeMessage", static_cast<const void*>(kGoodbyeMessage));

  int* secondsToSleepFor = new int();
  print_address("secondsToSleepFor", static_cast<const void*>(secondsToSleepFor));

  std::unique_ptr<unsigned long long> numberOfIterations = std::unique_ptr<unsigned long long>(
    new unsigned long long
  );
  print_address("numberOfIterations", static_cast<const void*>(numberOfIterations.get()));

  std::cout << kHelloMessage << std::endl;
  *secondsToSleepFor = 1;
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

void print_address(const char* name, const void* ptr_to_print_address_of){
  std::cout << name << " has address: " <<  ptr_to_print_address_of << std::endl;
}
