#ifndef _JKYI_MODULE_H_
#define _JKYI_MODULE_H_

#include"JKYi/stream.h"
#include"JKYi/singleton.h"
#include"JKYi/mutex.h"
#include"JKYi/protocol.h"

#include<map>
#include<unordered_map>

namespace JKYi{

//模块类，用于实现业务的动态加载
class Module{
public:
    enum Type{
        MODULE = 0,
    };

    typedef std::shared_ptr<Module> ptr;

    Module(const std::string& name,
            const std::string& version,
            const std::string& filename,
            uint32_t type = MODULE);

    virtual ~Module(){}

    virtual void onBeforeArgsParse(int argc,char ** argv);
    virtual void onAfterArgsParse(int argc,char ** argv);

    virtual bool onLoad();
    virtual bool onUnload();

    virtual bool onConnect(JKYi::Stream::ptr stream);
    virtual bool onDisconnect(JKYi::Stream::ptr stream);

    virtual bool onServerReady();
    virtual bool onServerUp();

    virtual bool handleRequest(JKYi::Message::ptr req,JKYi::Message::ptr rsp,JKYi::Stream::ptr stream);
    virtual bool handleNotify(JKYi::Message::ptr notify,JKYi::Stream::ptr stream);

    virtual std::string statusString();

    const std::string& getName()const { return m_name; }
    const std::string& getVersion()const { return m_version; }
    const std::string& getFilename()const { return m_filename; }
    const std::string& getId()const { return m_id; }

    void setFilename(const std::string& v) { m_filename = v; }
    uint32_t getType()const { return m_type; }
protected:
   std::string m_name;
   std::string m_version;
   //该模块的路径，也就相当于动态链接库的路径
   std::string m_filename;
   std::string m_id;
   uint32_t m_type;
};

class ModuleManager{
public:
    typedef RWMutex RWMutexType;
    
    ModuleManager();

    void add(Module::ptr m);
    void del(const std::string& name);
    void delAll();

    void init();

    Module::ptr get(const std::string& name);

    void onConnect(Stream::ptr stream);
    void onDisconnect(Stream::ptr stream);

    void listAll(std::vector<Module::ptr>& ms);
    void listByType(uint32_t type,std::vector<Module::ptr>& ms);
    void foreach(uint32_t type,std::function<void (Module::ptr)>cb);

private:
    void initModule(const std::string& path);
private:
    RWMutexType m_mutex; 
    //id - module
    std::unordered_map<std::string,Module::ptr> m_modules;
    // type - [id - module]
    std::unordered_map<uint32_t,std::unordered_map<std::string,Module::ptr> > m_type2Modules;
};

typedef JKYi::Singleton<ModuleManager> ModuleMgr;
}

#endif
