#include <string.h>
#include "spsc_queue.c"


spsc_queue_t test_q[MAX_NB_QUEUE];


int main()
{
  spsc_q_ptr = &(test_q[0]);
  init_spsc_queue();
  char text1[] = "Hello World  \n";
  char recv_txt[50];
  u32 rcv_len = 0;
  spsc_send_msg(text1,strlen(text1)+1,1);
  text1[12]='1';
  spsc_send_msg(text1,strlen(text1)+1,1);
  text1[12]='2';
  spsc_send_msg(text1,strlen(text1)+1,1);
  text1[12]='3';
  spsc_send_msg(text1,strlen(text1)+1,1);

  bool ret;
  ret = spsc_receive_msg(&(recv_txt[0]), &rcv_len, 1);
  printf("Recieved text: #%s#  of length %d returned %d \n",recv_txt, rcv_len, ret); 
  ret = spsc_receive_msg(&(recv_txt[0]), &rcv_len, 1);
  printf("Recieved text: #%s#  of length %d returned %d \n",recv_txt, rcv_len, ret); 
  ret = spsc_receive_msg(&(recv_txt[0]), &rcv_len, 1);
  printf("Recieved text: #%s#  of length %d returned %d \n",recv_txt, rcv_len, ret); 
  ret = spsc_receive_msg(&(recv_txt[0]), &rcv_len, 1);
  printf("Recieved text: #%s#  of length %d returned %d \n",recv_txt, rcv_len, ret); 

//read underflow
  ret = spsc_receive_msg(&(recv_txt[0]), &rcv_len, 1);
  printf("Recieved text: #%s#  of length %d returned %d \n",recv_txt, rcv_len, ret); 


// Add test for write overflow

// Add scenario - read underflow followed by write + read
// Add scenario - write overflow followed by write + read


  return 0;

}
