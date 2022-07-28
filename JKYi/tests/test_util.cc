#include"/home/admin/workSpace/JKYi/JKYi.h"
#include<assert.h>

static JKYi::Logger::ptr g_logger = JKYI_LOG_NAME("system");
void test_assert(){
    //JKYI_ASSERT(0); 
	JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"------------------------------";
	//JKYI_ASSERT2(0,"hello!!!!");
	JKYI_ASSERT(true);
	//JKYI_ASSERT(1);

    uint64_t now_ms = JKYi::GetCurrentMS();
    uint64_t now_us = JKYi::GetCurrentUS();
    JKYI_LOG_INFO(g_logger)<<" now_ms = "<<now_ms
                           <<" now_us = "<<now_us;
};
int main(int argc,char**argv){
    test_assert();
	return 0;
}
