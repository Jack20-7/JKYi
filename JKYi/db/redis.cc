#include"redis.h"
#include"JKYi/config.h"
#include"JKYi/log.h"
#include"JKYi/scheduler.h"
#include"JKYi/macro.h"

namespace JKYi{

static Logger::ptr g_logger = JKYI_LOG_NAME("system");
static JKYi::ConfigVar<std::map<std::string,std::map<std::string,std::string>>>::ptr 
         g_redis = JKYi::Config::Lookup("redis.config",
                    std::map<std::string,std::map<std::string,std::string>>(),
                    "redis config");

static std::string get_value(const std::map<std::string,std::string>& m,
                             const std::string& key,const std::string& def = ""){
    auto it = m.find(key);
    return it == m.end() ? def : it->second;
}

redisReply* RedisReplyClone(redisReply* r){
    redisReply * c = (redisReply*)calloc(1,sizeof(*r));
    c->type = r->type;
    switch(r->type){
        case REDIS_REPLY_INTEGER:
            c->integer = r->integer; 
            break;
        case REDIS_REPLY_ARRAY:
            if(r->element != nullptr && r->elements >0){
                c->element = (redisReply**)calloc(r->elements,sizeof(r));
                c->elements = r->elements;
                for(size_t i = 0;i < r->elements;++i){
                    c->element[i] = r->element[i];
                }
            }
            break;
        case REDIS_REPLY_ERROR:
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_STRING:
            if(r->str == nullptr){
                c->str = nullptr;
            }else{
                c->str = (char *)malloc(r->len + 1);
                memcpy(c->str,r->str,r->len);
                c->str[r->len] = '\0';
            }
            c->len = r->len;
            break;
    }
    return c;
}
Redis::Redis(){
    m_type = IRedis::REDIS;
}
Redis::Redis(const std::map<std::string,std::string>&conf){
    m_type = IRedis::REDIS;
    std::string tmp = get_value(conf,"host");
    auto pos = tmp.find(":");
    m_host = tmp.substr(0,pos);
    m_port = JKYi::TypeUtil::Atoi(tmp.substr(pos + 1));
    m_passwd = get_value(conf,"passwd");
    m_logEnable = JKYi::TypeUtil::Atoi(get_value(conf,"log_enable","1"));

    tmp = get_value(conf,"timeout_com");
    if(tmp.empty()){
        tmp = get_value(conf,"time_out");
    }
    uint64_t v = JKYi::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}
bool Redis::reconnect(){
    return redisReconnect(m_context.get());
}
bool Redis::connect(){
    return connect(m_host,m_port,50);
}
bool Redis::connect(const std::string& ip,int port ,uint64_t ms){
    m_host = ip;
    m_port = port;
    m_connectMs = ms;
    if(m_context){
        return true;
    }
    timeval tv = {(int)ms / 1000,(int)ms % 1000 * 1000};
    redisContext * c = redisConnectWithTimeout(ip.c_str(),port,tv);
    if(c){
        if(m_cmdTimeout.tv_sec || m_cmdTimeout.tv_usec){
            setTimeout(m_cmdTimeout.tv_sec * 1000 + m_cmdTimeout.tv_usec / 1000);
        }
        m_context.reset(c,redisFree);

        if(!m_passwd.empty()){
            auto r = (redisReply*)redisCommand(c,"auth %s",m_passwd.c_str());
            if(!r){
                JKYI_LOG_ERROR(g_logger) << "auth error :(" << m_host << " : "
                                         << m_port << " , " << m_name << " ) "; 
                return false;
            }
            if(r->type != REDIS_REPLY_STATUS){
                JKYI_LOG_ERROR(g_logger) << " auth reply type error : " << r->type;
                return false;
            }
            if(!r->str){
                JKYI_LOG_ERROR(g_logger) << " auth reply str error : NULL";
                return false;
            }
            if(!strcmp(r->str,"OK") == 0){
                return true;
            }else{
                JKYI_LOG_ERROR(g_logger) << " auth error : " << r->str;
                return false;
            }
        }
        return true;
    }
    return false;
}
void Redis::setTimeout(uint64_t v){
    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
    redisSetTimeout(m_context.get(),m_cmdTimeout);
}
ReplyPtr Redis::cmd(const char * fmt,...){
    va_list ap;
    va_start(ap,fmt);
    ReplyPtr rt = cmd(fmt,ap);
    va_end(ap);
    return rt;
}
ReplyPtr Redis::cmd(const char * fmt,va_list ap){
    auto r = (redisReply*)redisvCommand(m_context.get(),fmt,ap);
    if(!r){
        if(m_logEnable){
            JKYI_LOG_ERROR(g_logger) << " redisCommad error ";
        }
        return nullptr;
    }
    ReplyPtr rt(r,freeReplyObject);
    if(rt->type != REDIS_REPLY_ERROR){
        return rt;
    }
    if(m_logEnable){
        JKYI_LOG_ERROR(g_logger) << " redisCommad error:( " << fmt << " )( "
                                 << m_host << " : " << m_port << " )( " << m_name
                                 << " )" << " : " << r->str;
    }
    return nullptr;
}
ReplyPtr Redis::cmd(const std::vector<std::string>& argv){
    std::vector<const char *> v;
    std::vector<size_t> l;
    for(auto& i : argv){
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    auto r = (redisReply*) redisCommandArgv(m_context.get(),argv.size(),&v[0],&l[0]);
    if(!r){
        if(m_logEnable){
           JKYI_LOG_ERROR(g_logger) << " redisCommandArgv error ";
        }
        return nullptr;
    }
    ReplyPtr rt(r,freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR){
        return rt;
    }
    if(m_logEnable){
        JKYI_LOG_ERROR(g_logger) << " redisCommadArgv error:( "
                                 << m_host << " : " << m_port << " )( " << m_name
                                 << " )" << " : " << r->str;
    }
    return nullptr;
}
ReplyPtr Redis::getReply(){
    redisReply * r = nullptr;
    if(redisGetReply(m_context.get(),(void**)&r) == REDIS_OK){
        ReplyPtr rt(r,freeReplyObject);
        return rt;
    }
    if(m_logEnable){
        JKYI_LOG_ERROR(g_logger) << " redisGetReply error ";
    }
    return nullptr;
}
int Redis::appendCmd(const char * fmt,...){
    va_list ap;
    va_start(ap,fmt);
    int rt = appendCmd(fmt,ap);
    va_end(ap);
    return rt;
}
int Redis::appendCmd(const char * fmt,va_list ap){
    return redisAppendCommand(m_context.get(),fmt,ap);
}
int Redis::appendCmd(const std::vector<std::string>& argv){
    std::vector<const char * > v;
    std::vector<size_t> l;
    for(auto& i : argv){
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    return redisAppendCommandArgv(m_context.get(),argv.size(),&v[0],&l[0]);
}
RedisCluster::RedisCluster(){
    m_type = IRedis::REDIS_CLUSTER;
}
RedisCluster::RedisCluster(const std::map<std::string,std::string>&conf){
    m_type = IRedis::REDIS_CLUSTER;
    m_host = get_value(conf,"host");
    m_passwd = get_value(conf,"passwd");
    m_logEnable = JKYi::TypeUtil::Atoi(get_value(conf,"log_enable","1"));
    auto tmp = get_value(conf,"timeout_com");
    if(tmp.empty()){
        tmp = get_value(conf,"timeout");
    }
    uint64_t v = JKYi::TypeUtil::Atoi(tmp);
    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}
bool RedisCluster::reconnect(){
    return true;
}
bool RedisCluster::connect(){
    return connect(m_host,m_port,50);
}
bool RedisCluster::connect(const std::string& ip,int port,uint64_t ms){
    m_host = ip;
    m_port = port;
    m_connectMs = ms;
    if(m_context){
        return true;
    }
    timeval tv = {(int)ms / 1000,(int)ms % 1000 * 1000};
    auto c = redisClusterConnectWithTimeout(m_host.c_str(),tv,0);
    if(c){
        m_context.reset(c,redisClusterFree);
        if(!m_passwd.empty()){
            auto r = (redisReply*)redisClusterCommand(c,"auth %s",m_passwd.c_str());
            if(!r){
                JKYI_LOG_ERROR(g_logger) << " auth error";
                return false;
            }
            if(r->type == REDIS_REPLY_STATUS){
                JKYI_LOG_ERROR(g_logger) << " auth reply type error: " << r->type;
                return false;
            }
            if(!r->str){
                JKYI_LOG_ERROR(g_logger) << " auth reply str error: nullptr";
                return false;
            }
            if(strcmp(r->str,"OK") == 0){
                return true;
            }else{
                JKYI_LOG_ERROR(g_logger) << " auth error ,str = " << r->str;
                return false;
            }
        }
        return true;
    }
    return false;
}
void RedisCluster::setTimeout(uint64_t ms){
    return ;
}
ReplyPtr RedisCluster::cmd(const char * fmt,...){
    va_list ap;
    va_start(ap,fmt);
    ReplyPtr rt = cmd(fmt,ap);
    va_end(ap);
    return rt;
}
ReplyPtr RedisCluster::cmd(const char * fmt,va_list ap){
    redisReply * r = (redisReply*)redisClusterCommand(m_context.get(),fmt,ap);
    if(!r){
        if(m_logEnable){
            JKYI_LOG_ERROR(g_logger) << " redisClusterCommand error";
        }
        return nullptr;
    }
    ReplyPtr rt(r,freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR){
      return rt; 
    }
    if(m_logEnable){
        JKYI_LOG_ERROR(g_logger) << " redisClusterCommand error,reply type is error";
    }
    return nullptr;
}
ReplyPtr RedisCluster::cmd(const std::vector<std::string>& argv){
    std::vector<const char * > v;
    std::vector<size_t> l;
    for(auto& i : argv){
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    auto r = (redisReply*)redisClusterCommandArgv(m_context.get(),
                                    argv.size(),&v[0],&l[0]);
    if(!r){
        if(m_logEnable){
            JKYI_LOG_ERROR(g_logger) << " redisClusterCommandArgv error";
        }
        return nullptr;
    }
    ReplyPtr rt(r,freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR){
        return rt;
    }
    if(m_logEnable){
        JKYI_LOG_ERROR(g_logger) << " redisClusterCommand reply type is error";
    }
    return nullptr;
}
ReplyPtr RedisCluster::getReply(){
    redisReply * r = nullptr;
    if(redisClusterGetReply(m_context.get(),(void**)&r) == REDIS_OK){
        ReplyPtr rt(r,freeReplyObject);
        return rt;
    }
    if(m_logEnable){
        JKYI_LOG_ERROR(g_logger) << " redisClusterGetReply error";
    }
    return nullptr;
}
int RedisCluster::appendCmd(const char * fmt,...){
    va_list ap;
    va_start(ap,fmt);
    int rt = appendCmd(fmt,ap);
    va_end(ap);
    return rt;
}
int RedisCluster::appendCmd(const char * fmt,va_list ap){
    return redisClusterAppendCommand(m_context.get(),fmt,ap);
}
int RedisCluster::appendCmd(const std::vector<std::string>& argv){
    std::vector<const char *> v;
    std::vector<size_t> l;
    for(auto& i : argv){
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    return redisClusterAppendCommandArgv(m_context.get(),argv.size(),&v[0],&l[0]);
}
FoxRedis::FoxRedis(JKYi::FoxThread* thr,const std::map<std::string,std::string>& conf)
    :m_thread(thr),
     m_status(UNCONNECTED),
     m_event(nullptr){
     m_type = IRedis::FOX_REDIS;
     std::string tmp = get_value(conf,"host");
     auto pos = tmp.find(":");
     m_host = tmp.substr(0,pos);
     m_port = JKYi::TypeUtil::Atoi(tmp.substr(pos + 1));
     m_passwd = get_value(conf,"passwd");
     m_ctxCount = 0;
     m_logEnable = JKYi::TypeUtil::Atoi(get_value(conf,"log_enable","1"));

     tmp = get_value(conf,"timeout_com");
     if(tmp.empty()){
         tmp = get_value(conf,"timeout");
     }
     uint64_t v = JKYi::TypeUtil::Atoi(tmp);

     m_cmdTimeout.tv_sec = v / 1000;
     m_cmdTimeout.tv_usec = v % 1000 * 1000;
}

//该回调函数是当在进行密码认证的时候需要用到的
void FoxRedis::OnAuthCb(redisAsyncContext* c,void * rp,void * priv){
    FoxRedis* fr = (FoxRedis*)priv;
    redisReply * r = (redisReply*)rp; 
    if(!r){
        JKYI_LOG_ERROR(g_logger) << " auth error : reply is null";
        return ;
    }
    if(r->type != REDIS_REPLY_STATUS){
        JKYI_LOG_ERROR(g_logger) << " auth error : reply type is not redis_reply_status";
        return ;
    }
    if(!r->str){
        JKYI_LOG_ERROR(g_logger) << " auth error : reply str is nullptr";
        return ;
    }
    if(strcmp(r->str,"OK") == 0){
        JKYI_LOG_INFO(g_logger) << " auth ok : " << r->str << " ( " << fr->m_host
                                << " : " << fr->m_port << " , " << fr->m_name << " )";
        return ;
    }else{
        JKYI_LOG_ERROR(g_logger) << " auth error : " << r->str << " ( " << fr->m_host
                                 << " : " << fr->m_port << " , " << fr->m_name << " ) ";
    }
    return ;
}

//连接建立成功之后会调用的回调函数
void FoxRedis::ConnectCb(const redisAsyncContext * c,int status){
    FoxRedis * ar = static_cast<FoxRedis*>(c->data);
    if(!status){
        JKYI_LOG_INFO(g_logger) << " FoxRedis::ConnectCb "
                                << c->c.tcp.host << " : " << c->c.tcp.port
                                << " success ";
        ar->m_status = CONNECTED;
        if(!ar->m_passwd.empty()){
            int rt = redisAsyncCommand(ar->m_context.get(),FoxRedis::OnAuthCb,ar,
                                          "auth %s",ar->m_passwd.c_str());
            if(rt){
                JKYI_LOG_ERROR(g_logger) << " FoxRedis auth fail " << rt;
            }
        }
    }else{
        JKYI_LOG_ERROR(g_logger) << " FoxRedis Connectb " 
                                 << c->c.tcp.host << " : " << c->c.tcp.port
                                 << " fail,error : " << c->errstr;
        ar->m_status = UNCONNECTED;
    }
}
//断开连接时会调用的回调函数
void FoxRedis::DisconnectCb(const redisAsyncContext* c,int status){
    JKYI_LOG_INFO(g_logger) << " FoxRedis::DisconnectCb "
                            << c->c.tcp.host << " : " << c->c.tcp.port
                            << " status : " << status;
    FoxRedis * r = static_cast<FoxRedis*>(c->data);
    r->m_status = UNCONNECTED;
}

void FoxRedis::CmdCb(redisAsyncContext* ac,void * r,void * pridata){
    Ctx * ctx = static_cast<Ctx*>(pridata);
    if(!ctx){
        return ;
    }
    if(ctx->timeout){
        delete ctx;
        return ;
    }
     bool m_logEnable = ctx->rds->m_logEnable;
    redisReply * reply = (redisReply*)r;
    if(ac->err){
        if(m_logEnable){
           JKYI_LOG_ERROR(g_logger) << " redis cmd error,(" << ac->err << " ) "
                                    << ac->errstr;
        }
        if(ctx->fctx->fiber){
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }else if(!reply){
        if(m_logEnable){
            JKYI_LOG_ERROR(g_logger) << "redis cmd error  reply is null"; 
        }
        if(ctx->fctx->fiber){
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }else if(reply->type == REDIS_REPLY_ERROR){
        if(m_logEnable){
            JKYI_LOG_ERROR(g_logger) << " redis cmd error ";
        }
        if(ctx->fctx->fiber){
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }else{
        //执行到这里代表命令执行时成功的
        if(ctx->fctx->fiber){
            ctx->fctx->rpy.reset(RedisReplyClone(reply),freeReplyObject);
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }
    ctx->cancelEvent();
    delete ctx;
}
void FoxRedis::TimeCb(int fd,short event,void * d){
    FoxRedis * ar = static_cast<FoxRedis*>(d);
    JKYI_ASSERT(ar->m_thread == JKYi::FoxThread::GetThis());
    redisAsyncCommand(ar->m_context.get(),CmdCb,nullptr,"ping");
}
bool FoxRedis::init(){
    if(m_thread == JKYi::FoxThread::GetThis()){
        return pinit();
    }else{
        m_thread->dispatch(std::bind(&FoxRedis::pinit,this));
    }
    return true;
}
void FoxRedis::delayDelete(redisAsyncContext* c){
    return ;
}
bool FoxRedis::pinit(){
    if(m_status != UNCONNECTED){
        return true;
    }
    auto ctx = redisAsyncConnect(m_host.c_str(),m_port);
    if(!ctx){
        JKYI_LOG_ERROR(g_logger) << " redisAsyncConnect error";
        return false;
    }
    if(ctx->err){
        JKYI_LOG_ERROR(g_logger) << " redisAsyncConnect error : ctx->str = " << ctx->err; 
        return false;
    }
    ctx->data = this;
    redisLibeventAttach(ctx,m_thread->getBase());
    redisAsyncSetConnectCallback(ctx,ConnectCb);
    redisAsyncSetDisconnectCallback(ctx,DisconnectCb); 
    m_status = CONNECTING;
    m_context.reset(ctx,JKYi::nop<redisAsyncContext>);
    if(!m_event){
        m_event = event_new(m_thread->getBase(),-1,EV_TIMEOUT | EV_PERSIST,TimeCb,this);
        struct timeval tv = {120,0};
        evtimer_add(m_event,&tv);
    }
    TimeCb(0,0,this);
    return true;
}
ReplyPtr FoxRedis::cmd(const char * fmt,...){
    va_list ap;
    va_start(ap,fmt);
    ReplyPtr rt = cmd(fmt,ap);
    va_end(ap);
    return rt;
}

ReplyPtr FoxRedis::cmd(const char * fmt,va_list ap){
    char * buf = nullptr;
    //拼接出完成的命令
    int len = redisvFormatCommand(&buf,fmt,ap);
    if(len == -1){
        JKYI_LOG_ERROR(g_logger) << " redis fmt error : " << fmt;
        return nullptr;
    }
    FCtx fctx;
    fctx.cmd.append(buf,len);
    free(buf);
    fctx.scheduler = JKYi::Scheduler::GetThis();
    fctx.fiber = JKYi::Fiber::GetThis();

    m_thread->dispatch(std::bind(&FoxRedis::pcmd,this,&fctx));
    JKYi::Fiber::YieldToHold();
    JKYI_LOG_DEBUG(g_logger) << " cmd back";
    return fctx.rpy;
}
ReplyPtr FoxRedis::cmd(const std::vector<std::string>& argv){
    FCtx fctx;
    std::vector<const char * > args;
    std::vector<size_t> args_len;
    for(auto& i : argv){
        args.push_back(i.c_str());
        args_len.push_back(i.size());
    }
    char * buf = nullptr;
    int len = redisFormatCommandArgv(&buf,argv.size(),&(args[0]),&(args_len[0]));
    if(len == -1 || !buf){
        JKYI_LOG_ERROR(g_logger) << " redis fmt error";
        return nullptr;
    }
    fctx.cmd.append(buf,len);
    free(buf);

   fctx.scheduler = JKYi::Scheduler::GetThis();
   fctx.fiber = JKYi::Fiber::GetThis();

   m_thread->dispatch(std::bind(&FoxRedis::pcmd,this,&fctx));
   JKYi::Fiber::YieldToHold();

   return fctx.rpy;
}
void FoxRedis::pcmd(FCtx * fctx){
    if(m_status == UNCONNECTED){
        JKYI_LOG_INFO(g_logger) << " redis ( " << m_host << ": " << m_port << " ) "
                                << " unconnected";
        init();
        if(fctx->fiber){
            fctx->scheduler->schedule(&fctx->fiber);
        }
        return ;
    }
    Ctx * ctx(new Ctx(this));
    ctx->thread = m_thread;
    ctx->init();
    ctx->fctx = fctx;
    ctx->cmd = fctx->cmd;

    if(!ctx->cmd.empty()){
        redisAsyncFormattedCommand(m_context.get(),CmdCb,ctx,ctx->cmd.c_str()
                                         ,ctx->cmd.size());
    }
    return ;
}
FoxRedis::~FoxRedis(){
    if(m_event){
        evtimer_del(m_event);
        event_free(m_event);
    }
}
FoxRedis::Ctx::Ctx(FoxRedis* thr)
    :ev(nullptr),
     timeout(false),
     rds(thr),
     thread(nullptr){
    __sync_add_and_fetch(&rds->m_ctxCount,1);
}
FoxRedis::Ctx::~Ctx(){
    JKYI_ASSERT(thread == JKYi::FoxThread::GetThis());
    __sync_sub_and_fetch(&rds->m_ctxCount,1);
    if(ev){
        evtimer_del(ev);
        event_free(ev);
        ev = nullptr;
    }
}
void FoxRedis::Ctx::cancelEvent(){
    return ;
}
bool FoxRedis::Ctx::init(){
    ev = evtimer_new(rds->m_thread->getBase(),EventCb, this);
    evtimer_add(ev,&rds->m_cmdTimeout);
    return true;
}
void FoxRedis::Ctx::EventCb(int fd,short event,void * d){
    Ctx * ctx = static_cast<Ctx*>(d);
    ctx->timeout = 1;
    if(ctx->rds->m_logEnable){
    }
    if(ctx->fctx->fiber){
        ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
    }
    ctx->cancelEvent();
}

FoxRedisCluster::FoxRedisCluster(JKYi::FoxThread* thr,const std::map<std::string,
                                                       std::string>& conf)
    :m_thread(thr),
     m_status(UNCONNECTED),
     m_event(nullptr){
     m_ctxCount = 0;
     m_type = IRedis::FOX_REDIS_CLUSTER;
     m_host = get_value(conf,"host");
     m_passwd = get_value(conf,"passwd");
     m_logEnable = JKYi::TypeUtil::Atoi(get_value(conf,"log_enable","1"));

     std::string tmp = get_value(conf,"timeout_com");
     if(tmp.empty()){
         tmp = get_value(conf,"timeout");
     }
     uint64_t v = JKYi::TypeUtil::Atoi(tmp);

     m_cmdTimeout.tv_sec = v / 1000;
     m_cmdTimeout.tv_usec = v % 1000 * 1000;
}
void FoxRedisCluster::OnAuthCb(redisClusterAsyncContext* c,void * rp,void * priv){
    //FoxRedisCluster* fr = (FoxRedisCluster*)priv;
    redisReply * r = (redisReply*)rp;
    if(!r){
        JKYI_LOG_ERROR(g_logger) << " auth error : reply is null";
        return ;
    }
    if(r->type != REDIS_REPLY_STATUS){
        JKYI_LOG_ERROR(g_logger) << " authe error : reply->type = " << r->type;
        return ;
    }
    if(!r->str){
        JKYI_LOG_ERROR(g_logger) << " auth error : reply->str is null";
        return ;
    }
    if(strcmp(r->str,"OK") == 0){
        JKYI_LOG_INFO(g_logger) << "auth ok ! ";
        return ;
    }else{
        JKYI_LOG_ERROR(g_logger) << " auth error : str = " << r->str;
    }
    return ;
}
//TODO modify
void FoxRedisCluster::ConnectCb(const redisAsyncContext* c,int status){
    if(!status){
        JKYI_LOG_INFO(g_logger) << " FoxRedisCluster::ConnectCb (" << c->c.tcp.host 
                                << " : " << c->c.tcp.port << " )success";
    }else{
        JKYI_LOG_ERROR(g_logger) << " FoxRedisCluster::ConnectCb (" << c->c.tcp.host
                                 << " : " << c->c.tcp.port << " )fail,error = " 
                                 << c->errstr;
    }
    return ;
}
void FoxRedisCluster::DisconnectCb(const redisAsyncContext*c,int status){
    JKYI_LOG_INFO(g_logger) << " FoxRedisCluster::Disconnecb (" << c->c.tcp.host
                            << " : " << c->c.tcp.port  << " ) status = " << status;
}
void FoxRedisCluster::CmdCb(redisClusterAsyncContext* c,void * r,void * privdata){
    Ctx * ctx = static_cast<Ctx*>(privdata);
    if(ctx->timeout){
        delete ctx;
        return ;
    }
    auto m_logEnable = ctx->rds->m_logEnable;
    redisReply * reply = (redisReply*)r;
    if(c->err){
        if(m_logEnable){
            JKYI_LOG_ERROR(g_logger) << " redis cmd error ";
        }
        if(ctx->fctx->fiber){
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }else if(!reply){
        if(m_logEnable){
            JKYI_LOG_ERROR(g_logger) << " redis cmd error : reply is null";
        }
        if(ctx->fctx->fiber){
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }else if(reply->type == REDIS_REPLY_ERROR){
        if(m_logEnable){
            JKYI_LOG_ERROR(g_logger) << " redis cmd error : reply type is error,type = "
                                     << reply->type;
        }
        if(ctx->fctx->fiber){
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }else{
        //执行成功的话
        if(ctx->fctx->fiber){
            ctx->fctx->rpy.reset(RedisReplyClone(reply),freeReplyObject);
            ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
        }
    }
    delete ctx;
}
void FoxRedisCluster::TimeCb(int fd,short event,void * d){
    return ;
}
bool FoxRedisCluster::init(){
    if(m_thread == JKYi::FoxThread::GetThis()){
        return pinit();
    }else{
        m_thread->dispatch(std::bind(&FoxRedisCluster::pinit,this));
    }
    return true;
}
void FoxRedisCluster::delayDelete(redisClusterAsyncContext* c){
    return ;
}
bool FoxRedisCluster::pinit(){
    if(m_status != UNCONNECTED){
        return true;
    }
    redisClusterAsyncContext* ctx = 
                  (redisClusterAsyncContext*)malloc(sizeof(redisClusterAsyncContext));
    redisClusterAsyncSetConnectCallback(ctx,ConnectCb);
    redisClusterAsyncSetDisconnectCallback(ctx,DisconnectCb);
    redisClusterSetOptionAddNodes(ctx->cc,m_host.c_str());
    
    redisClusterLibeventAttach(ctx,m_thread->getBase());
    redisClusterConnect2(ctx->cc);
    if(ctx->cc->err){
        JKYI_LOG_ERROR(g_logger) << " Error :( " << ctx->cc->err << " ) " 
                                 << ctx->cc->errstr;
        return false;
    }
    m_status = CONNECTED;
    m_context.reset(ctx,JKYi::nop<redisClusterAsyncContext>);

    if(m_event){
        m_event = event_new(m_thread->getBase(),-1,EV_TIMEOUT | EV_PERSIST,TimeCb,this);
        struct timeval tv = {120,0};
        evtimer_add(m_event,&tv);
        TimeCb(0,0,this);
    }
    return true;
}
ReplyPtr FoxRedisCluster::cmd(const char * fmt,...){
    va_list ap ;
    va_start(ap,fmt);
    ReplyPtr rt = cmd(fmt,ap);
    va_end(ap);
    return rt;
}
ReplyPtr FoxRedisCluster::cmd(const char * fmt,va_list ap){
    char * buf = nullptr;
    int len = redisvFormatCommand(&buf,fmt,ap);
    if(len == 0 || !buf){
        JKYI_LOG_ERROR(g_logger) << " redis fmt error : " << fmt;
        return nullptr;
    }
    FCtx fctx;
    fctx.cmd.append(buf,len);
    free(buf);
    fctx.scheduler = JKYi::Scheduler::GetThis();
    fctx.fiber = JKYi::Fiber::GetThis();

    m_thread->dispatch(std::bind(&FoxRedisCluster::pcmd,this,&fctx));
    JKYi::Fiber::YieldToHold();
    return fctx.rpy;
}
ReplyPtr FoxRedisCluster::cmd(const std::vector<std::string>& argv){
    FCtx fctx ;

    std::vector<const char * > args;
    std::vector<size_t> args_len;
    for(auto& i : argv){
        args.push_back(i.c_str());
        args_len.push_back(i.size());
    }
    char * buf = nullptr;
    int len = redisFormatCommandArgv(&buf,argv.size(),&(args[0]),&(args_len[0]));
    if(len == -1 || !buf){
        JKYI_LOG_ERROR(g_logger) << " redis cmd error";
        return nullptr;
    }
    fctx.cmd.append(buf,len);
    free(buf);

    fctx.scheduler = JKYi::Scheduler::GetThis();
    fctx.fiber = JKYi::Fiber::GetThis();

    m_thread->dispatch(std::bind(&FoxRedisCluster::pcmd,this,&fctx));
    JKYi::Fiber::YieldToHold();
    return fctx.rpy;
}
void FoxRedisCluster::pcmd(FCtx* fctx){
    if(m_status != CONNECTED){
        JKYI_LOG_INFO(g_logger) << " redis (" << m_host << " ) unconnected";
        init();
        if(fctx->fiber){
            fctx->scheduler->schedule(&fctx->fiber);
        }
        return ;
    }
    Ctx * ctx(new Ctx(this));
    ctx->thread = m_thread;
    ctx->init();
    ctx->fctx = fctx;
    ctx->cmd = fctx->cmd;
    if(!ctx->cmd.empty()){
        redisClusterAsyncCommand(m_context.get(),CmdCb,ctx,&ctx->cmd[0],ctx->cmd.size());
    }
}
FoxRedisCluster::~FoxRedisCluster(){
    if(m_event){
        evtimer_del(m_event);
        event_free(m_event);
    }
}
FoxRedisCluster::Ctx::Ctx(FoxRedisCluster* r)
    :ev(nullptr),
     timeout(false),
     rds(r),
     thread(nullptr){
     fctx = nullptr;
     __sync_add_and_fetch(&rds->m_ctxCount,1);
}
FoxRedisCluster::Ctx::~Ctx(){
    JKYI_ASSERT(thread == JKYi::FoxThread::GetThis());
    __sync_sub_and_fetch(&rds->m_ctxCount,1);

    if(ev){
        evtimer_del(ev);
        event_free(ev);
        ev = nullptr;
    }
}
void FoxRedisCluster::Ctx::cancelEvent(){
    return ;
}
bool FoxRedisCluster::Ctx::init(){
    JKYI_ASSERT(thread == JKYi::FoxThread::GetThis());
    ev = evtimer_new(rds->m_thread->getBase(),EventCb,this);
    evtimer_add(ev,&rds->m_cmdTimeout);
    return true;
}
void FoxRedisCluster::Ctx::EventCb(int fd,short event,void * d){
    Ctx * ctx = static_cast<Ctx*>(d);
    if(!ctx->ev){
        return ;
    }
    ctx->timeout = 1;
    if(ctx->rds->m_logEnable){
    }
    ctx->cancelEvent();
    if(ctx->fctx->fiber){
        ctx->fctx->scheduler->schedule(&ctx->fctx->fiber);
    }
    return ;
}
IRedis::ptr RedisManager::get(const std::string& name){
    JKYi::RWMutex::WriteLock lock(m_mutex);
    auto it = m_datas.find(name);
    if(it == m_datas.end()){
       return nullptr;
    }
    if(it->second.empty()){
        return nullptr;
    }
    auto r = it->second.front();
    it->second.pop_front();
    if(r->getType() == IRedis::FOX_REDIS_CLUSTER || 
              r->getType() == IRedis::FOX_REDIS){
        it->second.push_back(r);
        return std::shared_ptr<IRedis>(r,JKYi::nop<IRedis>);
    }
    lock.unlock();
    auto rr = static_cast<ISyncRedis*>(r);
    if((time(0) - rr->getLastActiveTime()) > 30){
        if(!rr->cmd("ping")){
            if(!rr->reconnect()){
                JKYi::RWMutex::WriteLock lock(m_mutex);
                m_datas[name].push_back(r);
                return nullptr;
            }
        }
    }
    rr->setLastActiveTime(time(0));
    return std::shared_ptr<IRedis>(rr,std::bind(&RedisManager::freeRedis,this,
                                     std::placeholders::_1));
}
void RedisManager::freeRedis(IRedis* r){
    JKYi::RWMutex::WriteLock lock(m_mutex);
    m_datas[r->getName()].push_back(r);
}

RedisManager::RedisManager(){
    init();
}

void RedisManager::init(){
    m_config = g_redis->getValue();
    //已经创建的
    size_t done = 0;
    //一共需要创建的
    size_t total = 0;

    for(auto& i : m_config){
        auto type = get_value(i.second,"type");
        auto pool = JKYi::TypeUtil::Atoi(get_value(i.second,"pool"));
        auto passwd = get_value(i.second,"passwd");
        total += pool;
        for(int n = 0;n < pool;++n){
            if(type == "redis"){
                JKYi::Redis* rds(new Redis(i.second));
                rds->connect();
                rds->setLastActiveTime(time(0));
                JKYi::RWMutex::WriteLock lock(m_mutex);
                m_datas[i.first].push_back(rds);
                __sync_add_and_fetch(&done,1);
            }else if(type == "redis_cluster"){
                JKYi::RedisCluster* rds(new JKYi::RedisCluster(i.second));
                rds->connect();
                rds->setLastActiveTime(time(0));
                JKYi::RWMutex::WriteLock lock(m_mutex);
                m_datas[i.first].push_back(rds);
                __sync_add_and_fetch(&done,1);
            }else if(type == "fox_redis"){
                auto conf = i.second;
                std::string name = i.first;
                JKYi::FoxThreadMgr::GetInstance()->dispatch("redis",
                             [this,conf,name,&done](){
                JKYi::FoxRedis* rds(new JKYi::FoxRedis(JKYi::FoxThread::GetThis(),conf));
                rds->init();
                rds->setName(name);

                JKYi::RWMutex::WriteLock lock(m_mutex);
                m_datas[name].push_back(rds);
                __sync_add_and_fetch(&done,1);
                });
            }else if(type == "fox_redis_cluster"){
                auto conf = i.second;
                std::string name = i.first;
                JKYi::FoxThreadMgr::GetInstance()->dispatch("redis",
                             [this,conf,name,&done](){
                JKYi::FoxRedisCluster* rds(new 
                                JKYi::FoxRedisCluster(JKYi::FoxThread::GetThis(),conf));
                rds->init();
                rds->setName(name);

                JKYi::RWMutex::WriteLock lock(m_mutex);
                m_datas[name].push_back(rds);
                __sync_add_and_fetch(&done,1);
                });
            }else{
                __sync_add_and_fetch(&done,1);
            }
        }
    }
    while(done != total){
        usleep(5000);
    }
}

std::ostream& RedisManager::dump(std::ostream& os){
    os << "[RedisManager total = " << m_config.size() << " ]"  << std::endl;
    for(auto& i : m_config){
        os << "    " << i.first << " :[ ";
        for(auto& n : i.second){
            os << " {" << n.first << " : " <<n.second << " } ";
        }
        os << " ] " << std::endl;
    }
    return os;
}
ReplyPtr RedisUtil::Cmd(const std::string& name,const char * fmt,...){
    va_list ap;
    va_start(ap,fmt);
    ReplyPtr rt = Cmd(name,fmt,ap);
    va_end(ap);
    return rt;
}
ReplyPtr RedisUtil::Cmd(const std::string& name,const char * fmt,va_list ap){
    auto rds = RedisMgr::GetInstance()->get(name);
    if(!rds){
        return nullptr;
    }
    return rds->cmd(fmt,ap);
}

ReplyPtr RedisUtil::Cmd(const std::string& name,const std::vector<std::string>& args){
    auto rds = RedisMgr::GetInstance()->get(name);
    if(!rds){
        return nullptr;
    }
    return rds->cmd(args);
}
ReplyPtr RedisUtil::TryCmd(const std::string& name,uint64_t count,const char * fmt,...){
    for(uint32_t i = 0;i < count;++i){
        va_list ap;
        va_start(ap,fmt);
        ReplyPtr rt = Cmd(name,fmt,ap);
        va_end(ap);
        if(rt){
            return rt;
        }
    }
    return nullptr;
}
ReplyPtr RedisUtil::TryCmd(const std::string& name,uint64_t count,
                      const std::vector<std::string>& args){
    for(uint32_t i = 0;i < count;++i){
        ReplyPtr rt = Cmd(name,args);
        if(rt){
            return rt;
        }
    }
    return nullptr;
}
}
