/* 
Important to remember here that we are looking at a single allocating process
on a core - 
Unfortunately - this is required because on some processors the compare and swap
or equivalent instructions work on a per core basis
*/

#include <stdio.h>
#include <stdbool.h>
#define ALIGNMENT_PADDING 0
#define BUFFER_SIZE       256
#define PADDING_SIZE      16
#define PADDING_DATA      "########################################################################"
#define LOW_WATERMARK     20 /* Warn if less than 20% buffers are free */
#define WARNING(X)        (printf("Help! Only %d buffers are free!",X));


struct buff
{
char          buf      [BUFFER_SIZE];
char          padding  [PADDING_SIZE]; // To detect buffer overflows + enforce alignment restrictions(if any)
struct buff * next;
};

typedef struct buff buff_t;


struct buff_list
{
  buff_t  head;
  int     items;
  int     low_watermark;
};

typedef struct buff_list buff_list_t;
buff_list_t* free_list_ptr = NULL;

static inline bool compare_and_swap(void * adress, buff_t * oldval, buff_t * newval)
{
  return __sync_bool_compare_and_swap((int *)adress,oldval,newval);
}

static inline void accomodate_processor_idiosyncracies(void)
{
   /*This is not required on all processors but 
  compiler optimization + out of order execution can create undetectable bugs
  so we have a memory barrier */ 
//  __asm__("mfence")__;
}

void buff_init(void * mem_area_ptr, int size)
{
  free_list_ptr = mem_area_ptr;
  int space_for_control_struct = sizeof(buff_list_t) + ALIGNMENT_PADDING;
  buff_t *buffers = (buff_t *) (((char *)mem_area_ptr) + space_for_control_struct);
  free_list_ptr->head.next = buffers;

  int nb_buffers = (size - space_for_control_struct)/sizeof(buff_t);
  int i =0;
  buff_t * bufptr = buffers;
  for( ;i<nb_buffers-1;i++)
  {
     /* in the initial free list, next points to next contiguous buffer */
     bufptr[i].next = &(bufptr[i+1]);
     /* todo : initialize padding area */
  }
  bufptr[i].next = NULL;
  free_list_ptr->items = nb_buffers;
  free_list_ptr->low_watermark = (nb_buffers*LOW_WATERMARK)/100;
}

buff_t * buff_alloc()
{
    buff_t * buf;
    buff_t ** address_of_head_next ;
    buff_t * head_next_next ;
  /* Take a buffer from the free list*/
  do
  {
    address_of_head_next = &(free_list_ptr->head.next);
    buf  = *(address_of_head_next);
    head_next_next = buf->next;
    accomodate_processor_idiosyncracies();
  }while(false == compare_and_swap(address_of_head_next, buf ,head_next_next));
  
  /*Now update the number of free items */
  int nb_free_bufs = 0;
  do
  {
    nb_free_bufs = free_list_ptr->items;
  } while(false == compare_and_swap(&(free_list_ptr->items),nb_free_bufs, nb_free_bufs-1));

  /*
    Remember that the nb of free items may be off by a few buffers
    because other threads of execution may be updating as we speak
   */
  if(nb_free_bufs < free_list_ptr->low_watermark )
    WARNING(nb_free_bufs);
  return buf;
}

void buff_free(buff_t * buf_to_free)
{
    buff_t ** address_of_head_next ;
    buff_t * head_next ;
  /* Take a buffer from the free list*/
  do
  {
    address_of_head_next = &(free_list_ptr->head.next);
    head_next = *(address_of_head_next);
    buf_to_free->next = head_next;
    accomodate_processor_idiosyncracies();
  }while(false == compare_and_swap(address_of_head_next, head_next, buf_to_free ));
  
  /*Now update the number of free items */
  int nb_free_bufs = 0;
  do
  {
    nb_free_bufs = free_list_ptr->items;
  } while(false == compare_and_swap(&(free_list_ptr->items),nb_free_bufs, nb_free_bufs+1));

  return ;
}

