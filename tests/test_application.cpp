/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-23 09:38:22
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-23 10:13:29
 */ 
#include"../sylar/application.h"


int main(int argc, char *argv[])
{
    sylar::Application app;
    if(app.init(argc, argv)) {
        return app.run();
        
    }
    return 0;
}
