

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>



int main(int argc, char** argv) {
  

int size = 64;

  int i = 0;
  int size_classes[8] ={16, 32, 64, 128, 256, 512, 1024,2048};
  
  while(size > size_classes[i])
  {
    i++;
  }
  
  size = size_classes[i];
  printf("%d\n", size_classes[i]);
  return 0;
  }
		int x = ((log(size)/log(2))  - 4);
