# JKYi
高性能的C++服务器框架

## 1.日志模块+配置系统

对于JKYi的日志，我们可以在log.yml这个配置文件中对日志的输出方式进行配置，配置的默认格式为
```cpp
    - name: root
      level: info
      formatter: '%d%T%m%n'
      appender:
          - type: FileLogAppender
            file: root.txt
          - type: StdoutLogAppender
```

如果没有配置的话，默认的格式就是以上这种，使用的方式就是
```cpp
JKYI_LOG_INFO(JKYI_LOG_ROOT())<<"xxxxx"
JKYI_LOG_DEBUG(JKYI_LOG_ROOT())<<"xxxxx"
JKYI_LOG_ERROR(JKYI_LOG_ROOT())<<"xxxxx"
JKYI_LOG_FATAL(JKYI_LOG_ROOT())<<"xxxxx"
JKYI_LOG_WARN(JKYI_LOG_ROOT())<<"xxxxx"
```

通过上诉这种方式来输出不同日志级别的日志，但是要注意的是当前的配置文件中的日志级别是info，只有级别>=info的级别才会被输出。

对于日志的输出格式，定义的方式为
```cpp
m:日志消息体
p:日志级别
r:累计毫秒数
c:日志名称
t:线程id
n:换行
d:时间
f:文件名
l:行号
T:Tab
F:协程id
```
他们之间通过%号的形式进行间隔，用户可以通过修改配置文件中的formatter的方式来自定义输出的日志格式

配置文件中的appender表示的是日志的输出地,StdoutLogAppender表示的是之间输出到显示屏，FileLogAppender表示的是输出到日志文件，文件名通过fiie来指定

举例用法:

我们可以现在配置文件中写上我们自定义的格式，如:
```cpp
    - name: system
      level: debug
      formatter: '%d%T%m%n'
      appender:
          - type: FileLogAppender
            file: system.txt
            formatter: '%d%T[%p]%T%m%T%n'
          - type: StdoutLogAppender
```

加上之后，使用的方式就是
```cpp
JKYi::Logger:: g_system_log=JKYI_LOG_NAME("system");
//这里如果直接使用的话，由于配置文件还没有被加载，实际使用的还是root的格式
JKYI_LOG_INFO(g_system_log)<<"XXXX"

//然后如果我们想要使用我们自己新定义的配置的话，需要通过YAML来进行加载，也就是
YAML::Node& node=YAML::LoadFile("配置文件的路径");
JKYi::Config::LoadFromYaml(node)
//以上操作完成之后，如果在使用g_system_log打日志的话，就是根据我们用户自定义的配置
JKYI_LOG_INFO(g_system_log)<<"xxx"
//这样的话，输出到屏幕的日志格式就是'%d%T%m%n'，写入到system.txt的日志格式为'%d%T[%p]%T%m%T%n'
```



