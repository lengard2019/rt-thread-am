#include <am.h>
#include <klib.h>
#include <rtthread.h>

#define A0_IDX 10
#define arg0 c->gpr[A0_IDX]
rt_base_t g_from;
rt_base_t g_to;
static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
    case EVENT_YIELD: {
      if (g_from){
        *((Context **) g_from) = c;
      }
      return *((Context**) g_to);
    }
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) {
  g_to = to;
  yield();
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  g_to = to;
  g_from = from;
  yield();
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

typedef struct {
  void * tentry_;
  void * parameter_;
  void * texit_;
} warpper_param_t;

void warpper(void *arg) {
  warpper_param_t *param = (warpper_param_t *)arg;
  void (*tentry)(void *) = (void (*)(void *))param->tentry_;
  void (*texit)() = (void (*)())param->texit_;
  tentry(param->parameter_);

  texit();

}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  //make sure stack_addr is aligned to sizeof(uintptr_t)
  stack_addr = (uintptr_t)stack_addr & (sizeof(uintptr_t) - 1) ? 
    (rt_uint8_t *)(((uintptr_t)stack_addr + sizeof(uintptr_t) - 1) & ~(sizeof(uintptr_t) - 1)) :
    stack_addr;
  // TODO: find a way to construct Area.start
  // construct a warpper
  warpper_param_t *param = (warpper_param_t *)((uintptr_t)(stack_addr) - sizeof(warpper_param_t));
  stack_addr = (rt_uint8_t *)((uintptr_t)(stack_addr) - sizeof(warpper_param_t));
  // warpper_param_t *param = malloc(sizeof(warpper_param_t));
  param->tentry_ = tentry;
  param->parameter_ = parameter;
  param->texit_ = texit;
  Context *c = kcontext( (Area) {stack_addr - 2000, stack_addr}, warpper, param);
  return (rt_uint8_t *)c;
}
