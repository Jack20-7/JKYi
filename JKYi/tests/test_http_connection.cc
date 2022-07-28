#include<iostream>
#include<fstream>

#include"JKYi/http/http_connection.h"
#include"JKYi/log.h"
#include"JKYi/iomanager.h"
#include"JKYi/http/http_parser.h"

static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();
void test_pool(){
    JKYi::http::HttpConnectionPool::ptr pool(new JKYi::http::HttpConnectionPool(
            "www.sylar.top","",80,10,5,1000 * 30,false));
    JKYi::IOManager::GetThis()->addTimer(1000,[pool](){
            auto r = pool->doGet("/",300);
            JKYI_LOG_INFO(g_logger)<<r->toString();
            JKYI_LOG_INFO(g_logger)<<pool->getTotals();
        },true);
}
void run(){
    JKYi::IPAddress::ptr addr = JKYi::Address::LookupAnyIPAddress("www.sylar.top:80");
    if(!addr){
        JKYI_LOG_ERROR(g_logger)<<"get addr error";
        return ;
    }
    JKYi::Socket::ptr sock = JKYi::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt){
        JKYI_LOG_ERROR(g_logger)<<"connect "<<addr->toString()<<" fail";
        return ;
    }
    JKYi::http::HttpConnection::ptr conn(new JKYi::http::HttpConnection(sock));

    JKYi::http::HttpRequest::ptr req(new JKYi::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host","www.sylar.top");
    JKYI_LOG_INFO(g_logger)<<"req:"<<std::endl
                           <<req->toString();
    
    conn->sendRequest(req);
    auto rsp = conn->recvResponse();
    if(!rsp){
        JKYI_LOG_ERROR(g_logger)<<"recv response error";
        return ;
    }
    JKYI_LOG_INFO(g_logger)<<"rsp:"<<std::endl
                           <<rsp->toString();

    //将HttpResponse的信息写入到文件中去
    std::ofstream ofs("/var/rsp.dat");
    ofs << rsp->toString();

    JKYI_LOG_INFO(g_logger)<<"-----------------------------------------";

    auto r = JKYi::http::HttpConnection::DoGet("http://www.sylar.top/blog/",300);
    JKYI_LOG_INFO(g_logger)<<"result = "<<r->result
                           <<" error = "<<r->error
                           <<" rsp = " <<(r->rsp? r->rsp->toString() : "");
    JKYI_LOG_INFO(g_logger)<<" --------------------------------------------------------";

    test_pool();
}
int main(int argc,char ** argv){
    JKYi::IOManager iom(2);
    iom.schedule(run);
    return 0;
} 
