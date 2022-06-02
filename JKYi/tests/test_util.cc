#include"/home/admin/workSpace/JKYi/JKYi.h"
#include<assert.h>
void test_assert(){
    //JKYI_ASSERT(0); 
	JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"------------------------------";
	//JKYI_ASSERT2(0,"hello!!!!");
	JKYI_ASSERT(true);
	//JKYI_ASSERT(1);
};
int main(int argc,char**argv){
    test_assert();
	return 0;
}
