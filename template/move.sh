#!/bin/bash

#重新生成bin目录下的可执行文件和bin/module/下面的动态链接库

if [ ! -d bin/module ];then
    mkdir bin/module
else
    unlink bin/project_name
    unlink bin/module/libproject_name.so
fi

cp JKYi/bin/test_application bin/project_name
cp lib/libproject_name.so bin/module/
