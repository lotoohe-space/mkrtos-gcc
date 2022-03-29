#ifndef _ARM_H__
#define _ARM_H__
#include <type.h>


/**
* @breif 原子操作的变量
*/
typedef struct {
	volatile int32_t counter;
} Atomic_t;

/**
* @breif 原子变量自加
*/
 uint32_t atomic_inc(volatile Atomic_t* val);
/**
* @breif 原子变量自减
*/
 uint32_t atomic_dec(volatile Atomic_t* val);
/**
* @breif 设置原子变量的值
*/
 void atomic_set(volatile Atomic_t* val,uint32_t i);
/**
* 设置值，并返回之前的值
*/
 uint32_t atomic_ret_set(volatile Atomic_t* val,uint32_t i);
/**
* @breif 读取原子变量的值
*/
 uint32_t atomic_read(volatile Atomic_t* val);
/**
* @breif 加上一个数
*/ 
 void atomic_add(volatile Atomic_t* a,uint32_t val);
/**
* @breif 减去一个数
*/
 void AtomicSub(volatile Atomic_t* a,uint32_t val);
 uint32_t atomic_test(volatile Atomic_t* a,uint32_t val);
/**
* @breif 测试等于0，则进行赋值，并返回测试的结果
* @return 1等于0 ，0不等于0
*/
 uint32_t atomic_test_set(volatile Atomic_t* a,uint32_t val);
 /**
  * 设置val;并设置之前的结果，如果加之前是0，则返回1，否则返回0
  * @param a
  * @param val
  * @return
  */
uint32_t atomic_set_test(volatile Atomic_t* a,uint32_t val);
 /**
  * 自加1。并返回加之前的结果，如果加之前是0，则返回1，否则返回0
  * @param a
  * @return
  */
uint32_t atomic_inc_test(volatile Atomic_t* a);
/**
* @breif 测试等于0，则进行+1，并返回测试的结果；否则，不+1
* @return 1等于0 ，0不等于0
 */
uint32_t atomic_test_inc(volatile Atomic_t* a);
/**
* @breif 测试大于0，则减1;反之不动;
* @return 1：等于0 ，0：不等于0
*/
 uint32_t atomic_test_dec_nq(volatile Atomic_t* a);
/**
 * @brief 比较a，b的大小,如果b<=a a+=1，并且b<=a返回1，否则0
 *
 * @param a
 * @param b
 * @return uint32_t
 */
uint32_t atomic_cmp_hi_inc(volatile Atomic_t* a,volatile uint32_t b);

/**
 * @brief 比较a，b的大小,如果a<=b a+=1，并且a<=b返回1，否则0
 *
 * @param a
 * @param b
 * @return uint32_t
 */
uint32_t atomic_cmp_hi_inc1(volatile Atomic_t* a,volatile uint32_t b);



#endif
