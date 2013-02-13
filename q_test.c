/* Test code */

#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>

#include "spsc_queue.c"
int move_thread_to_core(int core_id) 
{
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);

   pthread_t current_thread = pthread_self();    
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int gl = 0;
void * thread_func_send(void * arg)
{
  struct timespec sleep_time,dummy_time_var;
  sleep_time.tv_sec = 0;
  
  int core_id = (int)arg;
  move_thread_to_core(core_id);

  char message[] = "message number  \n";
  for(unsigned int i = 100000 ;i>0; i--)
  {

    sleep_time.tv_nsec = rand()%500000;
    nanosleep(&sleep_time,&dummy_time_var);
    message[15] = 47 + (i%10);
    spsc_send_msg(message,17,1);
    printf("S: %s ", message);
  }
  printf("thread %d done\n",core_id);
}

void * thread_func_recv(void * arg)
{
  struct timespec sleep_time,dummy_time_var;
  sleep_time.tv_sec = 0;
  
  int core_id = (int)arg;
  move_thread_to_core(core_id);

  for(unsigned int i = 100000 ;i>0; i--)
  {
    char recv_msg[50];
    u32 len;
    sleep_time.tv_nsec = rand()%500000;
    nanosleep(&sleep_time,&dummy_time_var);
    while(can_spsc_queue_be_read(1))
    {
        spsc_receive_msg(recv_msg,&len,1);
        printf("R: %s ",recv_msg);
    }
  }
  printf("thread %d done\n",core_id);
}

#define ONE_MB (1024*1024)
int main()
{
  srand(42);
  void * mem_area = malloc(ONE_MB);
  spsc_q_ptr = mem_area;
  init_spsc_queue();
  pthread_t p1,p2;
  pthread_attr_t thd_attr;
  pthread_attr_init(&thd_attr);

  pthread_create(&p1,&thd_attr,thread_func_send,1);
  pthread_create(&p2,&thd_attr,thread_func_recv,2);

  pthread_join(p1, NULL);
  pthread_join(p2, NULL);


  return 0;
}
