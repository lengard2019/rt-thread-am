#include <am.h>
#include <klib.h>
#include <rtthread.h>

#define STACK_SIZE (1024 * 8)

// typedef struct {
//   void *tentry;
//   void *parameter;
//   void *texit;
// } args_t;

static void wrapper(void *arg) { // 包裹函数, 传入栈中的参数指针

  rt_ubase_t *stack_bottom = (rt_ubase_t *)arg;
  rt_ubase_t tentry = *stack_bottom; 
  stack_bottom--;
  rt_ubase_t texit = *stack_bottom; 
  stack_bottom--;
  rt_ubase_t parameter = *stack_bottom; 
  // 从tentry进入，从texit返回
  ((void(*)())tentry) (parameter);
  ((void(*)())texit) ();

}

static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
    case EVENT_YIELD:  {
      rt_thread_t cp = rt_thread_self(); 
      rt_ubase_t to = cp->user_data;
      c = *(Context**)to; // 
      break;
    }
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c; // 返回到a0寄存器
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) { // unsigned long

  rt_thread_t pcb = rt_thread_self();
  rt_ubase_t user_data_temp = pcb->user_data;
  pcb->user_data = to;
  yield();
  pcb->user_data = user_data_temp;

}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) { // to和from都是指向上下文指针变量的指针(二级指针)

  rt_thread_t pcb = rt_thread_self();
  rt_ubase_t user_data_temp = pcb->user_data;
  pcb->user_data = to;
  *(Context**)from = pcb->sp;
  yield();
  pcb->user_data = user_data_temp;

}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

// 创建一个上下文结构, 为每个上下文分配一个栈区
rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  
  uintptr_t stack_end = (uintptr_t)stack_addr;
  uintptr_t align_mask = sizeof(uintptr_t) - 1;
  uintptr_t aligned_end = stack_end & ~align_mask; // stack_addr强制8字节对齐

  // 存储线程的栈空间
  Area stack;
  stack.start = (rt_uint8_t*)aligned_end - STACK_SIZE;
  stack.end = (rt_uint8_t*)aligned_end;

  Context *cp = kcontext(stack, wrapper, (void *)(stack.end - sizeof(Context) - 8)); // context头，下面接着三个参数，注意对齐

  // 三个参数，由高至低分别是tentry,texit,parameter
  rt_ubase_t *stack_bottom = (rt_ubase_t *)(stack.end - sizeof(Context) - 8);
  *stack_bottom = (rt_ubase_t)tentry;
  stack_bottom--;
  *stack_bottom = (rt_ubase_t)texit;
  stack_bottom--;
  *stack_bottom = (rt_ubase_t)parameter;

  return (rt_uint8_t *)cp;

}
