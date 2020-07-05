/*
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-07-01 11:18:08
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-01 11:19:24
 */ 
#pragma once

#include <memory>
#include "module.h"

namespace sylar
{

class Library{

public:
    static Module::ptr GetModule(const std::string & path);
};


} // namespace sylar
