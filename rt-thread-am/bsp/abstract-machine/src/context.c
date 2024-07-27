#include <am.h>
#include <klib.h>
#include <rtthread.h>

//wrap tentry and texit and call them 
void fake_entry(void *arg){
	rt_ubase_t *arg_pointer = (rt_ubase_t *)arg;
	rt_ubase_t tentry = *arg_pointer;
	arg_pointer--;
	rt_ubase_t parameter = *arg_pointer; 
	arg_pointer--;
	rt_ubase_t texit = *arg_pointer; 
	//printf("wwwwwwwwwwwwwwwwwwww");
	((void(*)())tentry)((void*)parameter);
	printf("sggggggggggggggggg");
	((void(*)())texit)();
}

static Context* ev_handler(Event e, Context *c) {
	switch (e.event) {
		case EVENT_YIELD:{
			//printf("yield");
			rt_thread_t pcb = rt_thread_self(); 
    		rt_ubase_t to = pcb->user_data;
    		c = *(Context**)to;
			//c->mepc += 4;
		break;
		}
		case EVENT_IRQ_TIMER://native
    	break;
		default: printf("Unhandled event ID = %d\n", e.event); assert(0);
	}
	return c;
}

void __am_cte_init() {
  cte_init(ev_handler);
}

void rt_hw_context_switch_to(rt_ubase_t to) {
	rt_thread_t pcb = rt_thread_self();
	rt_ubase_t temp_user_data = pcb->user_data;
	pcb->user_data = to;
	yield();
	pcb->user_data = temp_user_data;
}

void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
	rt_thread_t pcb = rt_thread_self();
	from = pcb->user_data;
	pcb->user_data = to;
	yield();
	pcb->user_data = from;
}

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}

rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
	//printf("stack is %x\n", stack_addr);
	stack_addr = (rt_uint8_t*) ((uintptr_t)stack_addr & ~(sizeof(uintptr_t) - 1));
	/*new stack init*/
	Area new_stack;
	new_stack.end   = stack_addr;
	new_stack.start = stack_addr - 0x8000;
	//printf("stack start is %x, end is %x\n", new_stack.start, new_stack.end);
	/*new Context init and write in arg*/
	Context *new_conp = kcontext(new_stack, fake_entry, (void *)(new_stack.end - sizeof(Context)));
	/*read arg out*/
	rt_ubase_t *arg_pointer = (rt_ubase_t *)(new_stack.end - sizeof(Context));
	printf("context %x\n", sizeof(Context));
	*arg_pointer = (rt_ubase_t)tentry;
	arg_pointer--;
	*arg_pointer = (rt_ubase_t)parameter;
	arg_pointer--;
	*arg_pointer = (rt_ubase_t)texit;
	return (rt_uint8_t *)new_conp;
}
