# JKYi
高性能的C++服务器框架
##1.日志模块+配置系统
对于JKYi的日志，我们可以在log.yml这个配置文件中对日志的输出方式进行配置，配置的默认格式为
```cpp
    - name: root
      level: info
      formatter: '%d%T%m%n'
      appender:
          - type: FileLogAppender
            file: root.txt
          - type: StdoutLogAppender
```cpp

