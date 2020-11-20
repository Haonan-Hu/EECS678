#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]){
  fprintf(stdout, "MYVAR: %s\n", getenv("MYVAR"));
  // fprintf(stdout, "argv[1]: %s\n", argv[1]);
  return 0;
}
