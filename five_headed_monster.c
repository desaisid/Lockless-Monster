/* Test code for lockless allocator */

#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>

#include "lockless_alloc.c"

#define ONE_MB (1024*1024)

int move_thread_to_core(int core_id) 
{
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);

   pthread_t current_thread = pthread_self();    
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int stop_all_threads = 0;
void * alloc_dealloc_func(void * arg)
{
  struct timespec sleep_time,dummy_time_var;
  sleep_time.tv_sec = 0;
  
  int core_id = (int)arg;
  move_thread_to_core(core_id);

  buff_t * bufptr[2];
  for(unsigned int i = 100000 ;i>0; i--)
  {

    bufptr[0] = buff_alloc();
    bufptr[1] = buff_alloc();
    if(bufptr[0] == bufptr[1]) // something went wrong
    {
      stop_all_threads = 1;
      break;
    }
    // the next line will cause all running threads to stop
    // which is a decent thing to do when we are not doing well
    if (stop_all_threads == 1)
      break;
    sleep_time.tv_nsec = rand()%500000;
    nanosleep(&sleep_time,&dummy_time_var);
    buff_free(bufptr[0]);
    buff_free(bufptr[1]);
  }
  printf("thread %d done\n",core_id);
}

int main()
{
  srand(42); // Why 42? Because its the answer

  void * mem_area = malloc(ONE_MB);
  buff_init(mem_area,ONE_MB);

  pthread_t p1,p2,p3,p4,p5;
  pthread_attr_t thd_attr;
  pthread_attr_init(&thd_attr);

//clearly I have been lazy not to write a loop here :)
  pthread_create(&p1,&thd_attr,alloc_dealloc_func,1);
  pthread_create(&p2,&thd_attr,alloc_dealloc_func,2);
  pthread_create(&p3,&thd_attr,alloc_dealloc_func,3);
  pthread_create(&p4,&thd_attr,alloc_dealloc_func,4);
  pthread_create(&p5,&thd_attr,alloc_dealloc_func,5);

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);
  pthread_join(p3, NULL);
  pthread_join(p4, NULL);
  pthread_join(p5, NULL);


// if there is a sync issue, hopefully it will show in the first few links
  if(stop_all_threads == 1)
  {
     buff_t head = free_list_ptr->head;
     printf("list %p -> %p -> %p -> %p \n", &head , head.next, head.next->next, head.next->next->next);
  }
  return 0;
}
