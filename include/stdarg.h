 #ifndef _STDARG_H
 #define _STDARG_H
 
 typedef char *va_list;  // 定义va_list是一个字符指针类型。
 
 /* Amount of space required in an argument list for an arg of type TYPE.
    TYPE may alternatively be an expression whose type is used.  */
 /* 下面给出了类型为TYPE的arg参数列表所要求的空间容量。
    TYPE也可以是使用该类型的一个表达式 */
 
 // 下面这句定义了取整后的TYPE类型的字节长度值。是int长度(4)的倍数。
 #define __va_rounded_size(TYPE)  \
   (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
 
 // 下面这个宏初始化指针AP，使其指向传给函数的可变参数表的第一个参数。
 // 在第一次调用va_arg或va_end之前，必须首先调用va_start宏。参数LASTARG是函数定义
 // 中最右边参数的标识符，即'...'左边的一个标识符。AP是可变参数表参数指针，LASTARG是
 // 最后一个指定参数。&(LASTARG) 用于取其地址（即其指针），并且该指针是字符类型。加上
 // LASTARG的宽度值后AP就是可变参数表中第一个参数的指针。该宏没有返回值。
 // 第17行上的函数 __builtin_saveregs() 是在gcc的库程序libgcc2.c中定义的，用于保存
 // 寄存器。 相关说明参见 gcc手册“Target Description Macros”章中“Implementing the
 // Varargs Macros”小节。
 #ifndef __sparc__
 #define va_start(AP, LASTARG)                                           \
  (AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
 #else
 #define va_start(AP, LASTARG)                                           \
  (__builtin_saveregs (),                                                \
   AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
 #endif
 
 // 下面该宏用于被调用函数完成一次正常返回。va_end可以修改AP使其在重新调用
 // va_start之前不能被使用。va_end必须在va_arg读完所有的参数后再被调用。
 void va_end (va_list);          /* Defined in gnulib */  /* 在gnulib中定义 */
 #define va_end(AP)
 
 // 下面宏用于扩展表达式使其与下一个被传递参数具有相同的类型和值。
 // 对于缺省值，va_arg 可以用字符、无符号字符和浮点类型。在第一次使用 va_arg时，它返
 // 回表中的第一个参数，后续的每次调用都将返回表中的下一个参数。这是通过先访问AP，然
 // 后增加其值以指向下一项来实现的。va_arg 使用TYPE 来完成访问和定位下一项，每调用一
 // 次va_arg，它就修改AP以指示表中的下一参数。
 #define va_arg(AP, TYPE)                           \
  (AP += __va_rounded_size (TYPE),                  \
   *((TYPE *) (AP - __va_rounded_size (TYPE))))
 
 #endif /* _STDARG_H */
 

 
 