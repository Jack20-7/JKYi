#include"JKYi/db/mysql.h"
#include"JKYi/log.h"
#include"JKYi/config.h"
#include"JKYi/env.h"
#include"JKYi/macro.h"
#include"JKYi/iomanager.h"

#include<iostream>

static JKYi::Logger::ptr g_logger = JKYI_LOG_ROOT();
static JKYi::ConfigVar<std::map<std::string,std::string>>::ptr g_mysql_config = JKYi::Config::Lookup("mysql",std::map<std::string,std::string>(),"mysql config");

void printData(JKYi::ISQLData::ptr data){
    if(!data){
        JKYI_LOG_ERROR(g_logger) << "data is nullptr";
        return ;
    }
    std::cout << "----------------------------------------" << std::endl;
    int fields = data->getColumnCount();
    for(int i = 0;i < fields;++i){
        std::cout << data->getColumnName(i) << "\t";
    }
    std::cout << std::endl;
    while(data->next()){
        for(int i = 0;i < fields; ++i){
            std::cout << data->getString(i) << "\t";
        }
        std::cout << std::endl;
    }
    std::cout << "----------------------------------------" << std::endl;
    return ;
}
void run(){
    do{
        JKYi::Config::LoadFromConfDir("conf");
        JKYi::MySQL::ptr mysql = 
                         std::make_shared<JKYi::MySQL>(g_mysql_config->getValue());
        if(!mysql->connect()){
            JKYI_LOG_ERROR(g_logger) << "connect fail ";
            return ;
        }
        int rt = mysql->execute("insert into user(id,update_time) values(1,'%s')",
                                                           JKYi::Time2Str().c_str());
        JKYI_ASSERT(rt == 0);

        rt = mysql->execute("update user set update_time = %s where id = 1"
                                                            , "'2012-01-31 14:34:17'");
        JKYI_ASSERT(rt == 0);
        //测试stmt功能
        JKYi::MySQLStmt::ptr stmt = JKYi::MySQLStmt::Create(mysql,
                                        "update user set update_time = ? where id = 1");                                                
        stmt->bindString(1,JKYi::Time2Str(time(0)));

        rt = stmt->execute();
        JKYI_ASSERT(rt == 0);

        auto&& res = mysql->query("select * from user");
        printData(res);

        rt = mysql->execute("delete from user where id = 1");
        JKYI_ASSERT(rt == 0);

        printData(res);
        
    }while(false);
}

void test_mysql_mgr(){
    JKYi::Config::LoadFromConfDir("conf/");
    auto conn = JKYi::MySQLMgr::GetInstance()->get("test_0"); 
    if(!conn){
        JKYI_LOG_ERROR(g_logger) << "get conn error";
        return ;
    }
    JKYI_LOG_INFO(g_logger) << "insert data ---------------------";
    int rt = conn->execute("insert into user(id,update_time) values(1,'%s')",
                                                      JKYi::Time2Str(time(0)).c_str());
    JKYI_ASSERT(rt == 0);
    auto res = conn->query("select * from user");
    if(!res){
        JKYI_LOG_ERROR(g_logger) << "query result is null";
        return ;
    }
    printData(res);

    JKYI_LOG_INFO(g_logger) << " update data ---------------------";
    rt = conn->execute("update user set update_time = '%s' where id = 1",
                                                     "2000-01-01 13:00:00");
    JKYI_ASSERT(rt == 0);

    res = conn->query("select * from user");
    if(!res){
        JKYI_LOG_ERROR(g_logger) << "query result is null";
        return ;
    }
    printData(res);
   
    JKYI_LOG_INFO(g_logger) << "delete data----------------------";
    rt = conn->execute("delete from user where id = 1");
    JKYI_ASSERT(rt == 0);
    res = conn->query("select * from user");
    if(!res){
        JKYI_LOG_ERROR(g_logger) << "query result is null";
        return ;
    }
    printData(res);

}
int main(int argc,char ** argv){
    JKYi::EnvMgr::GetInstance()->init(argc,argv);
    JKYi::IOManager iom;
    //iom.schedule(&run);
    iom.schedule(test_mysql_mgr);
    return 0;
}
