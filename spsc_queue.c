#include <stdio.h>
#include <stdbool.h>

#define MAX_MSG_LEN         100
#define MAX_NB_MSG_IN_QUEUE 20
#define MAX_NB_QUEUE        4

typedef unsigned int u32;

struct spsc_msg
{
  u32  len;
  char buf[MAX_MSG_LEN];
};

typedef struct spsc_msg spsc_msg_t;

struct spsc_queue
{
  u32   read_idx                       ;
  u32   write_idx                      ;
  spsc_msg_t queue     [MAX_NB_MSG_IN_QUEUE];
};

typedef struct spsc_queue spsc_queue_t;

spsc_queue_t * spsc_q_ptr;
static inline bool is_spsc_q_write_allowed(u32 write_idx, u32 read_idx)
{
  return ((write_idx + 1)%MAX_NB_MSG_IN_QUEUE) != read_idx ;   
}

static inline bool is_spsc_q_read_allowed(u32 write_idx, u32 read_idx)
{
  return (( read_idx )%MAX_NB_MSG_IN_QUEUE) != write_idx;
}

void init_spsc_queue()
{
  if(NULL == spsc_q_ptr)
    return;
  spsc_q_ptr->read_idx  = 0;
  spsc_q_ptr->write_idx = 1;
}

bool spsc_send_msg(char * sndbuf,u32 len, u32 qidx)
{
  u32 writen_idx    =  spsc_q_ptr[qidx].write_idx;
  u32 read_done_idx =  spsc_q_ptr[qidx].read_idx;
  // is it ok to write
  if (!is_spsc_q_write_allowed(writen_idx, read_done_idx))
    return false;  
  //then write
  u32 to_write_idx  = (writen_idx+1)%MAX_NB_MSG_IN_QUEUE;
  spsc_msg_t * dst_msg   =  &(spsc_q_ptr[qidx].queue[to_write_idx]);
  memcpy(dst_msg->buf, sndbuf, len);
  dst_msg->len = len;
  spsc_q_ptr[qidx].write_idx = to_write_idx; 
  return true;
} 

bool spsc_receive_msg(char *rcvbuf, u32 *len, u32 qidx)
{
  u32 writen_idx    =  spsc_q_ptr[qidx].write_idx;
  u32 read_done_idx =  spsc_q_ptr[qidx].read_idx;
  *len = 0;
  // is it ok to read
  if (!is_spsc_q_read_allowed(writen_idx, read_done_idx))
    return false;
  // then read
  u32 to_read_idx   =  (read_done_idx+1)%MAX_NB_MSG_IN_QUEUE;
  spsc_msg_t *rcv_msg    =  &(spsc_q_ptr[qidx].queue[to_read_idx]);
  memcpy(rcvbuf, rcv_msg->buf, rcv_msg->len);
  *len              =  rcv_msg->len;
  spsc_q_ptr[qidx].read_idx++;// = to_read_idx;
  return true;
}

static inline bool can_spsc_queue_be_read(u32 qidx)
{
  u32 writen_idx    =  spsc_q_ptr[qidx].write_idx;
  u32 read_done_idx =  spsc_q_ptr[qidx].read_idx;
  return is_spsc_q_read_allowed(writen_idx, read_done_idx);
}
