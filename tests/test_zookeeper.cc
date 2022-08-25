#include"JKYi/zk_client.h"
#include"JKYi/log.h"
#include"JKYi/iomanager.h"

#include<vector>

static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();

int g_argc;

void on_watcher(int type,int stat,const std::string& path,JKYi::ZKClient::ptr client){
    JKYI_LOG_INFO(g_logger) << " type = " <<type
                            << " stat = " <<stat
                            << " path = " <<path
                            << " fiber = " <<JKYi::Fiber::GetThis()
                            << " iomanager = " <<JKYi::IOManager::GetThis();
    if(stat == ZOO_CONNECTED_STATE){
        //当连接建立成功时
        //std::vector<std::string> vals;
        //Stat stat;
        //int rt = client->getChildren("/",vals,true,&stat);
        //if(rt == ZOK){
        //    JKYI_LOG_INFO(g_logger) << "[" << JKYi::Join(vals.begin(),vals.end(),",")
        //                            << "]";
        //}else{
        //    JKYI_LOG_INFO(g_logger) << "getChildren error " << rt;
        //}

        int rt = 0;
        std::string new_path;
        new_path.resize(255);
        rt = client->create("/zkxxx","",new_path,&ZOO_OPEN_ACL_UNSAFE,
                              ZOO_EPHEMERAL);
        if(rt == ZOK){
            JKYI_LOG_INFO(g_logger) << "[ " << new_path.c_str() << " ]";
        }else{
            JKYI_LOG_INFO(g_logger) << "create znode error";
        }
        rt = client->create("/zkxxx","",new_path,&ZOO_OPEN_ACL_UNSAFE,
                              ZOO_EPHEMERAL | ZOO_SEQUENCE);
        if(rt == ZOK){
            JKYI_LOG_INFO(g_logger) << "create node [ " <<new_path.c_str() << "]";
        }else{
            JKYI_LOG_INFO(g_logger) << "create node error";
        }

        rt = client->get("/hello",new_path,true);
        if(rt == ZOK){
            JKYI_LOG_INFO(g_logger) << "get [ " << new_path.c_str() << "]";
        }else{
            JKYI_LOG_INFO(g_logger) << "get error";
        }
        rt = client->create("/hello","",new_path,&ZOO_OPEN_ACL_UNSAFE,ZOO_EPHEMERAL);
        if(rt == ZOK){
            JKYI_LOG_INFO(g_logger) << "create node [" << new_path.c_str() << "]";
        }else{
            JKYI_LOG_INFO(g_logger) << "create node error";
        }
        rt = client->set("/hello","xxx");
        if(rt == ZOK){
            JKYI_LOG_INFO(g_logger) << "set [ " << new_path.c_str() << "]";
        }else{
            JKYI_LOG_INFO(g_logger) << "set error";
        }
        rt = client->del("/hello");
        if(rt == ZOK){
            JKYI_LOG_INFO(g_logger) << "del node [ " << new_path.c_str() << "]";
        }else{
            JKYI_LOG_INFO(g_logger) << "del error";
        }
        
    }else if(ZOO_EXPIRED_SESSION_STATE){
        client->reconnect();
    }
}

int main(int argc,char ** argv){
    g_argc = argc;
    JKYi::IOManager iom;
    JKYi::ZKClient::ptr client(new JKYi::ZKClient);

    JKYI_LOG_INFO(g_logger) << client->init("127.0.0.1:2181",5000,on_watcher);
    iom.addTimer(1115000,[client](){client->close();});

    iom.stop();

    return 0;
}
