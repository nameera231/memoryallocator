#define _GNU_SOURCE

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

size_t xxmalloc_usable_size(void* ptr);

//element in  freelist
struct freelist_element{
  void* next;
} typedef element_t;

//free list in form of linked list
//(pointer to head of list)
typedef struct freelist{
  int size_class;
  element_t* head;
} freelist_t;


//header struct to store size class in page
typedef struct header{
  int magic;
  int size;
} header_t;

//magic number will be used for making sure we are at the right location in the page
#define MAGIC_NUMBER 23451


//array containing freelists of all size classes. 

freelist_t lists[8] = {

 {
  .size_class = 16,
	.head = NULL
},
 {
  .size_class = 32,
	.head = NULL
},
 {
  .size_class = 64,
	.head = NULL
},
 {
  .size_class = 128,
	.head = NULL
},
{
  .size_class = 256,
	.head = NULL
},
 {
  .size_class = 512,
	.head = NULL
},
{
  .size_class = 1024,
	.head = NULL
},
 {
  .size_class = 2048,
	.head = NULL
}

};
// The minimum size returned by malloc
#define MIN_MALLOC_SIZE 16

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// The size of a single page of memory, in bytes
#define PAGE_SIZE 0x1000

// A utility logging function that definitely does not call malloc or free
void log_message(char* message);

/**
 * Allocate space on the heap.
 * \param size  The minimium number of bytes that must be allocated
 * \returns     A pointer to the beginning of the allocated space.
 *              This function may return NULL when an error occurs.
 */
void* xxmalloc(size_t size) {

  //if size requested is greater than size class
  if (size > 2048)
  {
     // Round the size up to the next multiple of the page size
    size = ROUND_UP(size, PAGE_SIZE);

    // Request memory from the operating system in page-sized chunks
    void* p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  // Check for errors
    if (p == MAP_FAILED) {
      log_message("mmap failed! Giving up.\n");
      exit(2);
    }

    return p;
  }
	else
  {

    //rounding to obtain size class for size
    int i = 0;
    int size_classes[8] ={16, 32, 64, 128,256, 512, 1024,2048};
    
    while(size > size_classes[i])
    {
      i++;
    }
    
    size = size_classes[i];

    // get the index inside freelist array
    int index = ((log(size)/log(2))  - 4);
  
    void* final_pointer = lists[index].head;

    //if the final pointer finds a chunk of memory 
    if (final_pointer != NULL)
    {
      lists[index].head = lists[index].head->next;
      return final_pointer;
    }
    //if the final pointer cannot find a chunk of memory in freelist
    else
    {
       
      //insert memory into free list of that size class
      void* new_chunk = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      if (new_chunk == MAP_FAILED)
      { 
        log_message("mmap failed01! Giving up.\n");
        exit(2);
      }
      
      //set the header for allocated page
      header_t* temp = (header_t*)new_chunk;
      temp->size = size;
      temp->magic = MAGIC_NUMBER;
    
      
      lists[index].head = (element_t*)((uint8_t*)new_chunk + size);
      element_t* cursor = lists[index].head;
       
       //break allocated page into chunks of size equal to the relevant size class
      for(int i = 1; i < ((PAGE_SIZE/size) - 1); i++)
          //indented too far
     {
            //make each chunk in page an element in freelist and have it point to the next chunk in the page
            cursor->next = (element_t*)((uint8_t*)lists[index].head + (i*size));
            cursor = cursor->next;
     }
 
      final_pointer = lists[index].head;
      lists[index].head = lists[index].head->next;
      return (void*)final_pointer;
    }
	}
}

 


/**
 * Free space occupied by a heap object.
 * \param ptr   A pointer somewhere inside the object that is being freed
 */
void xxfree(void* ptr) {
  // Don't free NULL!
  if (ptr == NULL) return;
  //round down by the page to see what size it was
  size_t size = xxmalloc_usable_size(ptr);
  if(size == 0){
    return;
  }
  //round ptr down to find the beginning of the freed object
  element_t* size_info = (element_t*)((intptr_t)(ptr - ( (intptr_t)ptr % size)));
  int index = ((log(size)/log(2))  - 4);
  size_info->next = lists[index].head;
  lists[index].head =  size_info;
  return;
}

/**
 * Get the available size of an allocated object. This function should return the amount of space
 * that was actually allocated by malloc, not the amount that was requested.
 * \param ptr   A pointer somewhere inside the allocated object
 * \returns     The number of bytes available for use in this object
 */
size_t xxmalloc_usable_size(void* ptr) {
  // If ptr is NULL always return zero
  intptr_t cursor = (intptr_t)ptr;
  
  if (ptr == NULL) {
    return 0;
  }

  // round down by page size to locate relevant header
  header_t* size_info = (header_t*)(cursor - ( cursor % PAGE_SIZE));
  
  
  //check header for magic number and check array for size. Return size if magic number is correct
  if(MAGIC_NUMBER != size_info->magic)
  {
    //magic number is incorrect so return
    return 0;
  }
//magic number matched so return the size
  return size_info->size;
}

/**
 * Print a message directly to standard error without invoking malloc or free.
 * \param message   A null-terminated string that contains the message to be printed
 */
void log_message(char* message) {
  // Get the message length
  size_t len = 0;
  while (message[len] != '\0') {
    len++;
  }

  // Write the message
  if (write(STDERR_FILENO, message, len) != len) {
    // Write failed. Try to write an error message, then exit
    char fail_msg[] = "logging failed\n";
    write(STDERR_FILENO, fail_msg, sizeof(fail_msg));
    exit(2);
  }
}
