#include <stdio.h>

int main(int argc, char * argv[]) {
  printf("Hello from child process! Args: ");
  for(int i = 0; i < argc; i++) {
    printf("%s ", argv[i]);
  }
  printf("\n");
  return 0;
}

