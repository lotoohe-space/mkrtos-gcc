#ifndef _SPIN_LOCK_H__
#define _SPIN_LOCK_H__

#include <arch/atomic.h>

/**
* @breif 
*/
typedef struct spin_lock{
	/*互斥量锁标志*/
	Atomic_t lock;
}*pspinlock_handler,spinlock_handler;

/**
* @breif 自旋锁创建
*/
pspinlock_handler spin_lock_create(void);
/**
 * @brief 销毁自旋锁
 * 
 * @param pslh 
 */
void spin_lock_destory(pspinlock_handler pslh);
/**
* @breif 自旋锁初始化
* @breif 需要初始化的自旋锁
*/
void spin_lock_init(pspinlock_handler pslh);
/**
* @breif 锁上
* @param psh 上锁的自旋锁
*/
int32_t spin_lock(pspinlock_handler psh);
/**
* @breif 解锁
* @param psh 解锁的自旋锁
*/ 
int32_t spin_unlock(pspinlock_handler psh);

#endif
