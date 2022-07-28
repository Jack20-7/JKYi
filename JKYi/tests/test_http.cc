#include"../JKYi/JKYi.h"
static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();
void test_request(){
    JKYi::http::HttpRequest::ptr req(new JKYi::http::HttpRequest());
    req->setHeader("host","www.baidu.com");
    req->setBody("hello,JKYi");
    req->dump(std::cout)<<std::endl;
}
void test_response(){
    JKYi::http::HttpResponse::ptr rsp(new JKYi::http::HttpResponse());
    rsp->setHeader("X-X","JKYi");
    rsp->setBody("hello,JKYi");
    rsp->setStatus((JKYi::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout)<<std::endl;
}
int main(int argc,char ** argv){
    test_request();
    JKYI_LOG_INFO(g_logger)<<"---------------------";
    test_response();
    return 0;
}
