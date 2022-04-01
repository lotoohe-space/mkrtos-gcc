/**
* @breif 自旋锁只用在系统只有同优先级任务时，如果只存在一个高优先级任务，则自旋锁将导致任务卡死。
* @author 1358745329@qq.com
*/
#include <type.h>
#include <ipc/spin_lock.h>
#include <mkrtos/mem.h>
/**
* @breif 自旋锁创建
*/
pspinlock_handler spin_lock_create(void){
    pspinlock_handler psh;
	psh=OSMalloc(sizeof(spin_lock));
	if(psh==NULL){
		return psh;
	}
    atomic_set(&(psh->lock),0);
	return psh;
}
/**
* @breif 自旋锁初始化
* @breif 需要初始化的自旋锁
*/
void spin_lock_init(pspinlock_handler pslh){
	if(pslh==NULL){
		return ;
	}
    atomic_set(&(pslh->lock),0);
}
/**
 * @brief 销毁自旋锁
 * 
 * @param pslh 
 */
void spin_lock_destory(pspinlock_handler pslh){
	if(pslh==NULL){
		return ;
	}
	OSFree(pslh);
}
/**
* @breif 锁上
* @param psh 上锁的自旋锁
*/
int32_t spin_lock(pspinlock_handler psh){
	if(psh==NULL){
		return -1;
	}
again:
	if(!atomic_test_set(&(psh->lock),1)){
		/*加锁失败，挂起任务，休眠并等待被解锁*/
		goto again;
	}else{
        /*原子操作加锁成功*/
	}
	return 0;
}
/**
* @breif 解锁
* @param psh 解锁的自旋锁
*/ 
int32_t spin_unlock(pspinlock_handler psh){
	if(psh==NULL){
		return -1;
	}
	atomic_set(&(psh->lock),0);
	return 0;
}
