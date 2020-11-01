/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-06-22 21:41:13
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-13 18:53:15
 */

#include "application.h"

#include <unistd.h>

#include <iostream>
#include <vector>

#include "config.h"
#include "daemon.h"
#include "env.h"
#include "http/http_server.h"
#include "iomanager.h"
#include "log.h"
#include "sylar/http/ws_server.h"
#include "sylar/module.h"
#include "sylar/tcp_server.h"
#include "sylar/worker.h"

namespace sylar {

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static ConfigVar<std::string>::ptr g_server_work_path = Config::Lookup(
    "server.work_path", std::string("/home/zsj/workspace/work/sylar"),
    "server work path");

static ConfigVar<std::string>::ptr g_server_pid_file = Config::Lookup(
    "server.pid_file", std::string("sylar.pid"), "server pid file");


static sylar::ConfigVar<std::vector<TcpServerConf>>::ptr g_servers_conf =
    sylar::Config::Lookup("servers", std::vector<TcpServerConf>(),
                          "http server config");
Application* Application::s_instance = nullptr;

Application::Application() { s_instance = this; }

bool Application::init(int argc, char** argv) {
    SYLAR_LOG_INFO(g_logger) << "init function begin";

    m_argc = argc, m_argv = argv;
    sylar::EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    sylar::EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    sylar::EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
    sylar::EnvMgr::GetInstance()->addHelp("p", "print help");

    bool is_print_help = false;

    if (!sylar::EnvMgr::GetInstance()->init(argc, argv)) {
        is_print_help = true;
    }

    if (sylar::EnvMgr::GetInstance()->has("p")) {
        is_print_help = true;
    }


    std::string conf_path = sylar::EnvMgr::GetInstance()->getConfigPath();
    SYLAR_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    sylar::Config::LoadFromConfDir(conf_path);
    ModuleMgr::GetInstance()->init();
    std::vector<Module::ptr> modules;

    //取出当前加载的模块
    ModuleMgr::GetInstance()->listAll(modules);

    //调用所有模块在参数解析之前的函数
    for (auto i : modules) {
        i->onBeforeArgsParse(argc, argv);
    }

    if (is_print_help) {
        sylar::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    //调用所有模块在参数解析之后的函数
    for (auto i : modules) {
        i->onAfterArgsParse(argc, argv);
    }

    //清空元素   减少每一个对象的引用计数
    modules.clear();


    int run_type = 0;
    if (sylar::EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if (sylar::EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }

    //未选择运行模式（直接运行或者守护进程）  返回
    if (run_type == 0) {
        sylar::EnvMgr::GetInstance()->printHelp();
        return false;
    }

    //保存当前线程运行信息(线程号）的文件： /home/zsj/workspace/work/sylar + ‘/'
    //+ sylar.pid
    std::string pidfile =
        g_server_work_path->getValue() + "/" + g_server_pid_file->getValue();
    SYLAR_LOG_INFO(g_logger) << pidfile;

    //判断当前有无进程占用此进程号运行
    if (sylar::FSUtil::IsRunningPidfile(pidfile)) {
        SYLAR_LOG_ERROR(g_logger) << "server is runing: " << pidfile;
        return false;
    }

    // /home/zsj/workspace/work/sylar
    if (!sylar::FSUtil::Mkdir(g_server_work_path->getValue())) {
        SYLAR_LOG_ERROR(g_logger)
            << "create work path [" << g_server_work_path->getValue()
            << " errno=" << errno << " errstr=" << strerror(errno);
        // std::cout << __FILE__ << " : " << __LINE__ << "create work path
        // faile" <<std::endl;
        return false;
    }

    // SYLAR_LOG_INFO(g_logger) << "end of init function!";
    return true;
}

bool Application::run() {
    // 是否为守护进程方式运行
    bool is_daemon = sylar::EnvMgr::GetInstance()->has("d");
    return start_daemon(m_argc, m_argv,
                        std::bind(&Application::main, this,
                                  std::placeholders::_1, std::placeholders::_2),
                        is_daemon);
}


int Application::main(int argc, char** argv) {
    // SYLAR_LOG_DEBUG(g_logger) << pidfile;
    SYLAR_LOG_DEBUG(g_logger) << "run main";
    std::string conf_path = sylar::EnvMgr::GetInstance()->getConfigPath();
    sylar::Config::LoadFromConfDir(conf_path, true);
    {
        //将当前运行的进程id写入进程信息文件，标志已经在运行
        std::string pidfile = g_server_work_path->getValue() + "/" +
            g_server_pid_file->getValue();
        std::ofstream ofs(pidfile);
        if (!ofs) {
            SYLAR_LOG_ERROR(g_logger)
                << "open pidfile " << pidfile << " failed";
            return false;
        }
        ofs << getpid();
    }


    // sylar::IOManager iom;
    // iom.schedule(std::bind(&Application::run_fiber,this));
    // iom.stop();

    m_mainIOManager.reset(new IOManager(1, true, "main"));
    m_mainIOManager->schedule(std::bind(&Application::run_fiber, this));
    m_mainIOManager->addTimer(
        2000, []() {}, true);
    m_mainIOManager->stop();
    return 0;
}

int Application::run_fiber() {
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
    bool has_error = false;
    for (auto& i : modules) {
        if (!i->onLoad()) {
            SYLAR_LOG_ERROR(g_logger) << "module name=" << i->getName()
                                      << " version=" << i->getVersion()
                                      << " filename=" << i->getFilename();
            has_error = true;
        }
    }
    if (has_error) {
        _exit(0);
    }


    WorkerMgr::GetInstance()->init();
    auto http_confs = g_servers_conf->getValue();
    for (auto& i : http_confs) {
        std::vector<Address::ptr> address;
        for (auto& a : i.address) {
            size_t pos = a.find(":");
            if (pos == std::string::npos) {
                SYLAR_LOG_ERROR(g_logger) << "invalid address:" << a;
                continue;
            }
            auto addr = Address::LookupAny(a);
            if (addr) {
                address.push_back(addr);
                continue;
            }

            std::vector<std::pair<Address::ptr, uint32_t>> result;
            if (sylar::Address::GetInterfaceAddress(result, a.substr(0, pos))) {
                // SYLAR_LOG_ERROR(g_logger) << "invalid address: "<<a;

                for (auto& x : result) {
                    auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                    if (ipaddr) {
                        ipaddr->setPort(atoi(a.substr(pos + 1).c_str()));
                    }
                    address.push_back(ipaddr);
                }
                continue;
            }
            auto aaddr = Address::LookupAny(a);
            if (aaddr) {
                address.push_back(aaddr);
                continue;
            }
            SYLAR_LOG_ERROR(g_logger) << "invalid address " << a;
            _exit(0);
        }
        IOManager* accept_worker = IOManager::GetThis();
        IOManager* process_worker = IOManager::GetThis();
        if (!i.accept_worker.empty()) {
            accept_worker =
                WorkerMgr::GetInstance()->getAsIOManager(i.accept_worker).get();
            if (!accept_worker) {
                SYLAR_LOG_ERROR(g_logger)
                    << "accept_worker: " << i.accept_worker << "not exites";
                _exit(0);
            }
        }
        if (!i.process_worker.empty()) {
            process_worker = WorkerMgr::GetInstance()
                                 ->getAsIOManager(i.process_worker)
                                 .get();
            if (!process_worker) {
                SYLAR_LOG_ERROR(g_logger)
                    << "process_worker:" << i.process_worker << "not exists";
                _exit(0);
            }
        }
        // http::HttpServer::ptr server(new http::HttpServer(i.keepalive));
        // http::HttpServer::ptr server(new
        // http::HttpServer(i.keepalive,process_worker,accept_worker));

        TcpServer::ptr server;
        if (i.type == "http") {
            server.reset(new sylar::http::HttpServer(
                i.keepalive, process_worker, accept_worker));
        } else if (i.type == "ws") {
            server.reset(
                new sylar::http::WSServer(process_worker, accept_worker));
        } else {
            SYLAR_LOG_ERROR(g_logger)
                << "invalid server type=" << i.type
                << LexicalCast<TcpServerConf, std::string>()(i);
            _exit(0);
        }
        std::vector<Address::ptr> fails;
        if (!server->bind(address, fails, i.ssl)) {
            for (auto& x : fails) {
                SYLAR_LOG_ERROR(g_logger) << "bind address fail:" << *x;
            }
            _exit(0);
        }
        if (i.ssl) {
            if (!server->loadCertificates(i.cert_file, i.key_file)) {
                SYLAR_LOG_ERROR(g_logger)
                    << "loadCertificates fail, cert_file=" << i.cert_file
                    << " key_file=" << i.key_file;
            }
        }
        if (!i.name.empty()) {
            server->setName(i.name);
        }
        server->setConf(i);
        server->start();
        // m_httpservers.push_back(server);
        m_servers[i.type].push_back(server);
    }

    for (auto& i : modules) {
        i->onServerReady();
    }
    return 0;
}

bool Application::getServer(const std::string& type,
                            std::vector<TcpServer::ptr>& svrs) {
    auto it = m_servers.find(type);
    if (it == m_servers.end()) {
        return false;
    }
    svrs = it->second;
    return true;
}

}  // namespace sylar
