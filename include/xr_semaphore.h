//信号量

#include <xr_util.h>

namespace xr {
struct semaphore_t{
    sem_t sem;
    int init(){
        if (0 != sem_init(&this->sem, 0, 0)){
           perror("semaphore initialization failed");
           return FAIL;
        }
        return SUCC;
    }
    inline void wait(){
        ::sem_wait(&this->sem);
    }
    inline void post(){
        ::sem_post(&this->sem);
    }
    void destory(){
        ::sem_destroy(&this->sem);
    }
    virtual ~semaphore_t(){
        this->destory();
    }
};

}//end namespace xr