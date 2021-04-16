#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>
#include <sys/uio.h>
#include <sstream>
#include <string.h>
#include <iomanip>

namespace
{
  volatile std::sig_atomic_t gSignalStatus;
}

void signal_handler(int signal)
{
  gSignalStatus = signal;
}

int get_first_pid_of_process(const char* processname);
void hex_dump_from_remote_process(pid_t pid, void* starting_address, size_t count);
bool read_intvalue_from_process(pid_t pid, void* starting_address, int& value);
bool write_intvalue_to_process(pid_t pid, void* starting_address, int value);
int bytes_to_int(
  unsigned char b0,
  unsigned char b1,
  unsigned char b2,
  unsigned char b3
);
void int_to_bytes(
  int value,
  unsigned char &b0,
  unsigned char &b1,
  unsigned char &b2,
  unsigned char &b3
);
void printerror();

int main(int argc, char **argv){
  std::signal(SIGINT, signal_handler);

  std::cout << "Welcome to 'fiddler'. A memory fiddler." << std::endl;
  std::cout << "Specify a process name and memory address." << std::endl;
  int pid = 0;
  void* address_to_watch = nullptr;
  if(argc > 2){
    pid = get_first_pid_of_process(static_cast<const char*>(argv[1]));
    address_to_watch = (void *)strtol(argv[2], nullptr , 0);
  }
  if(pid > 0 && address_to_watch != nullptr){
    std::cout << "Will fiddle with: " << pid << std::endl;
    std::cout << "Address to watch: " << address_to_watch << std::endl;
  }

  std::cout << "start [Send SIGINT to Terminate]" <<std::endl;
  while(gSignalStatus != SIGINT){
    std::time_t result = std::time(nullptr);
    std::cout << std::ctime(&result);
    if(pid > 0 && address_to_watch != nullptr){
      //hex_dump_from_remote_process(static_cast<pid_t>(pid), address_to_watch, 8);
      int val = 0;
      if(read_intvalue_from_process(static_cast<pid_t>(pid), address_to_watch, val)){
        std::cout << "value of int at " << address_to_watch << " is " << val << std::endl;
        if(val > 50){
          //try to knock it back down to zero
          write_intvalue_to_process(static_cast<pid_t>(pid), address_to_watch, 0);
        }
      }
    }
    std::this_thread::sleep_for (std::chrono::seconds(1));
  }
  std::cout << "Goodbye from 'fiddler'." << std::endl;

  return 0;
}

int get_first_pid_of_process(const char* processname){
  std::ostringstream ss;
  char pidline[1024];
  char *pid;
  int i =0;
  int pid_number = 0;

  ss << "pidof " << processname;
  auto fp = popen(ss.str().c_str(),"r");
  if(nullptr == fp){
    std::cout << "popen failed. Could not get PID." << std::endl;
    return -1;
  }
  if(nullptr == fgets(pidline,1024,fp)){
    pclose(fp);
    std::cout << "fgets failed. Could not get PID." << std::endl;
    return -1;
  }
  std::cout << "PIDs for '" << processname << "' are (will return 1st): " << pidline;
  pid = strtok (pidline," ");
  if(nullptr == pid){
    std::cout << "strtok failed. Could not get PID." << std::endl;
    return -1;
  }
  pid_number = atoi(pid);
  pclose(fp);
  return pid_number;
}

void hex_dump_from_remote_process(pid_t pid, void* starting_address, size_t count){
  const size_t kSize = 8;
  struct iovec local[1];
  struct iovec remote[1];
  char buf1[kSize];

  local[0].iov_base = buf1;
  local[0].iov_len = kSize;

  remote[0].iov_base = starting_address;
  remote[0].iov_len = kSize;

  auto nread = process_vm_readv(pid, local, 1, remote, 1, 0);
  if(nread > 0){
    std::cout << nread << " bytes read from " << pid << ": ";
    for(ssize_t index=0; index < nread; index++){
      unsigned int value = static_cast<unsigned int>(buf1[index]);
      std::cout << std::setfill('0') << std::setw(2) << std::hex << (0xff & value);
    }
    std::cout << std::endl;
  }else if(0 == nread){
    std::cout << "no bytes read but no error either" << std::endl;
  }else{
    printerror();
  }
}

bool read_intvalue_from_process(pid_t pid, void* starting_address, int& value){
  const size_t kSize = 4;
  struct iovec local[1];
  struct iovec remote[1];
  char buf1[kSize];
  local[0].iov_base = buf1;
  local[0].iov_len = kSize;
  remote[0].iov_base = starting_address;
  remote[0].iov_len = kSize;
  auto nread = process_vm_readv(pid, local, 1, remote, 1, 0);
  if(kSize == nread){
    unsigned char b[4];
    b[0] = static_cast<unsigned char>(buf1[0]);
    b[1] = static_cast<unsigned char>(buf1[1]);
    b[2] = static_cast<unsigned char>(buf1[2]);
    b[3] = static_cast<unsigned char>(buf1[3]);
    value = bytes_to_int(b[0], b[1], b[2], b[3]);
    return true;
  }else if(0 == nread){
    std::cout << "no bytes read but no error either" << std::endl;
    return false;
  }else{
    printerror();
    return false;
  }
}

bool write_intvalue_to_process(pid_t pid, void* starting_address, int value){
  const size_t kSize = 4;
  struct iovec local[1];
  struct iovec remote[1];
  char buf1[kSize]{0};
  buf1[0] = value & 0xFF;
  buf1[1] = (value>8) & 0xFF;
  buf1[2] = (value>16) & 0xFF;
  buf1[3] = (value>32) & 0xFF;
  local[0].iov_base = buf1;
  local[0].iov_len = kSize;
  remote[0].iov_base = starting_address;
  remote[0].iov_len = kSize;
  std::cout << "try to write " << value << " to " << starting_address << std::endl;
  auto nwrite = process_vm_writev(pid, local, 1, remote, 1, 0);
  if(kSize == nwrite){
    return true;
  }else if(0 == nwrite){
    std::cout << "no bytes written but no error either" << std::endl;
    return false;
  }else{
    printerror();
    return false;
  }
}

int bytes_to_int(
  unsigned char b0,
  unsigned char b1,
  unsigned char b2,
  unsigned char b3
){
  return (b3 << 24) | (b2 << 16) | (b1 << 8) | (b0);
}

void int_to_bytes(
  int value,
  unsigned char &b0,
  unsigned char &b1,
  unsigned char &b2,
  unsigned char &b3
){
  b0 = value & 0xFF;
  b1 = (value>8) & 0xFF;
  b2 = (value>16) & 0xFF;
  b3 = (value>32) & 0xFF;
}

void printerror(){
  switch (errno) {
      case EINVAL:
        std::cout << "ERROR: INVALID ARGUMENTS." << std::endl;
        break;
      case EFAULT:
        std::cout << "ERROR: UNABLE TO ACCESS TARGET MEMORY ADDRESS." << std::endl;
        break;
      case ENOMEM:
        std::cout << "ERROR: UNABLE TO ALLOCATE MEMORY." << std::endl;
        break;
      case EPERM:
        std::cout << "ERROR: INSUFFICIENT PRIVILEGES TO TARGET PROCESS." << std::endl;
        break;
      case ESRCH:
        std::cout << "ERROR: PROCESS DOES NOT EXIST." << std::endl;
        break;
      default:
        std::cout << "ERROR: Unknown: " << errno << std::endl;
  }
}
