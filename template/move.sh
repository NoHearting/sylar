###
 # @Descripttion: 
 # @version: 
 # @Author: zsj
 # @Date: 2020-07-01 11:03:26
 # @LastEditors: zsj
 # @LastEditTime: 2020-07-05 13:15:12
### 
!/bin/sh

# unlink sylar/bin/sylar
# unlink bin/module/libproject_name.so

if [ ! -d bin/module ]
then 
    mkdir bin/module
else
    unlink bin/project_name
    unlink bin/module/libproject_name.so
fi

cp sylar/bin/sylar bin/project_name
cp lib/libproject_name.so bin/module