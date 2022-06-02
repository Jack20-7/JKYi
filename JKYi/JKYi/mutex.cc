#include"mutex.h"

namespace JKYi{
   //构造函数
Semaphore::Semaphore(uint32_t count){
   if(sem_init(&m_semaphore,0,count)){
	   throw std::logic_error("sem_init error");
   }
}
   //析构函数
Semaphore::~Semaphore(){
    if(sem_destroy(&m_semaphore)){
		throw std::logic_error("sem_destroy error");
	}
}
   //等待信号量
void Semaphore::wait(){
   if(sem_wait(&m_semaphore)){
	   throw std::logic_error("sem_wait error");
   }
}
   //释放信号量
void Semaphore::notify(){
    if(sem_post(&m_semaphore)){
       throw std::logic_error("sem_post error");
	}
}


}
