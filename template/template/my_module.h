/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-07-01 11:02:34
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-01 17:29:10
 */ 
#pragma once

#include "sylar/module.h"

namespace name_space
{

class MyModule : public sylar::Module{
public:
    typedef std::shared_ptr<MyModule> ptr;

    MyModule();

    bool onLoad() override;
    bool onUnload() override;
    bool onServerReady()override;
    bool onServerUp()override;
};
    
} // namespace name_space
