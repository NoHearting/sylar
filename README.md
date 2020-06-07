<!--
 * @Descripttion: 
 * @version: 
 * @Author: zsj
 * @Date: 2020-06-06 23:45:02
 * @LastEditors: zsj
 * @LastEditTime: 2020-06-07 23:32:03
--> 
# sylar(C++高性能服务器框架)
## 主要模块

### [1、日志模块](#log)
### [2、配置模块](#config)


### <span id="log">日志模块</span>
#### 1、简介
- 支持流式风格和格式化风格写日志
- 支持日志格式自定义
- 支持定义日志级别，多日志分离等功能
- 支持通过配置文件配置日志系统（目前支持yaml）
#### 2、组件
* 日志管理器，用于管理系统中的日志器
* 日志器，用于日志的格式化输出
* 日志级别，定义日志的级别
* 日志输出器，用于日志的具体输出，输出到文件或者是标准输出
* 日志格式器，格式化日志的输出形式
* 日志事件，用于输出当前日志的信息（时间、线程id...）
#### 3、使用
##### 流式风格
```cpp
SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "test log root";
SYLAR_LOG_INFO(SYLAR_LOG_NAME("system")) << "test log system";


/**
 * SYLAR_LOG_ROOT() 取出一个日志器,名字为root
 * SYLAR_LOG_NAME("system") 取出一个名为system的日志器，没有就创建
 * SYLAR_LOG_INFO(looger) 使用一个日志器打印INFO级别的日志信息，想要打印不同级别日志，使用SYLAR_LOG_{LEVEL}(logger)
 */
 
```

#### 格式化风格
```cpp
SYLAR_LOG_FMT_INFO(SYLAR_LOG_ROOT(),"%s","test log test");
SYLAR_LOG_FMT_INFO(SYLAR_LOG_Name(),"%s","test log system");

/**
 * SYLAR_LOG_ROOT() 取出一个日志器,名字为root
 * SYLAR_LOG_NAME("system") 取出一个名为system的日志器，没有就创建
 * SYLAR_LOG_FMT_INFO(logger,fmt,...)使用一个日志器打印fmt格式的日志，想要打印不用级别日志，使用SYLAR_LOG_FMT_{LEVEL}(logger,fmt,...)
 */

```

#### 4、配置
```yaml
# log.yml
logs:
  - name: root
    level: info
    formatter: "%d%T%m%n"
    appenders:
      - type: FileLogAppender
        file: root.txt
      - type: StdoutLogAppender
  - name: system
    level: error
    formatter: "%d%T%m%n"
    appenders:
      - type: FileLogAppender
        file: system.txt
        formatter: "%d%T[%p]%T%m%n"
      - type: StdoutLogAppender

# 配置了两个logger，root和system
#   root：
#       日志级别为：INFO
#       日志格式为："%d%T%m%n"
#       有两个日志输出器：FileLogAppender 和 StdoutLogAppender，分别输出到文件和控制台
#   system同理

```

### <span id="config">配置模块</span>
#### 1、简介
#### 2、组件