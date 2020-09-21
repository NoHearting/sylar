/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-07-01 18:04:15
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 19:49:52
 */ 
#include "sylar/application.h"

int main(int argc, char ** argv) {
    sylar::Application app; 
    if(app.init(argc,argv)){
        return app.run();
    }

    return 0;
}