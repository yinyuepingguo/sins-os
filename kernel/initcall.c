#include <sins/init.h>

void do_initcalls(initcall_t *begin, initcall_t *end)
{
	result_t res;
	printk("begin = %p, end = %p\n", begin, end);
	while (begin != end) {
		printk("handler = %p, name = %s\n", 
			begin->handler, begin->name);
		res = begin->handler();
		if (unlikely(FAILED(res))) {
			panic("initcall function: %s failed, error code %d.\n",
				begin->name, res);
		}
		begin++;	
	}
}
