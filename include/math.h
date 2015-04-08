#ifndef _MATH_H
#define _MATH_H

#include <type_check.h>

#define min(x,y) ({ \
	typeof(x) _x = x;	\
	typeof(y) _y = y;	\
        TYPE_SAME(x, y);            \
        _x < _y ? _x : _y; })

#define max(x,y) ({ \
	typeof(x) _x = x;	\
	typeof(y) _y = y;	\
       	TYPE_SAME(x, y);            \
        _x > _y ? _x : _y; })

#define abs(x) ((x) > 0? (x): -(x))

#define swap(a, b)	\
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

#define align_up(num, size)	(((num)+((size)-1))/(size) * (size))
#define align_down(num, size) ((num)/(size) * (size))

/*#define ceil_div(num, div)	({	\
	typeof(num) _num = (num);	\
	typeof(div) _div = (div);	\
	long __res = _num / _div;	\
	if (_num % _div)	\
		__res++;	\
	__res;	\
 })*/

#define ceil_div(num, div) ({	\
	int __res = num / div;	\
	if (num % div)	\
		__res++;	\
	__res;})

#endif
