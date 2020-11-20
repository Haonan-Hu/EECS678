#include <iostream>
#include <unistd.h>

int main(int argc, char* argv[])
{
  std::cout << "Inside Hello World\n";
  std::cout << "argv[1]: " << argv[1] << "\n";

  if(atoi(argv[1]) == 1)
  {
    sleep(10);
  }
  return(0);
}
