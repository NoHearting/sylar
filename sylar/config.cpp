/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-06-06 14:23:07
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-13 18:50:55
 */
#include "config.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "env.h"
#include "log.h"

namespace sylar {


static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

/**
 * @brief 递归读取配置文件中的配置到一个list中
 */
static void ListAllMember(
    const std::string& prefix, const YAML::Node& node,
    std::list<std::pair<std::string, const YAML::Node>>& output) {
    if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") !=
        std::string::npos) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT())
            << "Config invalid name:" << prefix << ":" << node;
        return;
    }
    output.push_back(std::make_pair(prefix, node));
    if (node.IsMap()) {
        for (auto& iter : node) {
            ListAllMember(
                (prefix.empty() ? iter.first.Scalar()
                                : (prefix + "." + iter.first.Scalar())),
                iter.second, output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;

    ListAllMember("", root, all_nodes);
    for (auto& item : all_nodes) {
        std::string key = item.first;
        if (key.empty()) {
            continue;
        }
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::ptr var =
            LookupBase(key);  //查找配置是否存在，有则覆盖，无则添加
        if (var) {
            if (item.second.IsScalar()) {
                var->fromString(item.second.Scalar());
            } else {
                std::stringstream ss;
                ss << item.second;
                var->fromString(ss.str());
            }
        }
    }
}


static std::map<std::string, uint64_t> s_fileToModifytime;
static sylar::Mutex s_mutex;

void Config::LoadFromConfDir(const std::string& path, bool force) {
    std::string absolute_path =
        sylar::EnvMgr::GetInstance()->getAbsolutePath(path);
    std::vector<std::string> files;
    FSUtil::listAllFile(files, absolute_path, ".yml");
    // SYLAR_LOG_DEBUG(g_logger) << "files size="<<files.size();
    for (auto& i : files) {
        // SYLAR_LOG_DEBUG(g_logger) << "file=" << i;
        {
            struct stat st;
            lstat(i.c_str(), &st);
            sylar::Mutex::Lock lock(s_mutex);
            if (!force && s_fileToModifytime[i] == (uint64_t)st.st_mtime) {
                continue;
            }
            s_fileToModifytime[i] = (uint64_t)st.st_mtime;
        }
        try {
            // SYLAR_LOG_DEBUG(g_logger) << i;
            YAML::Node root = YAML::LoadFile(i);
            LoadFromYaml(root);
            SYLAR_LOG_DEBUG(g_logger)
                << "LoadConfFile file=" << i << " success!!";
        } catch (...) {
            SYLAR_LOG_ERROR(g_logger)
                << "LoadConfFile file=" << i << " failed!";
        }
    }
}

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    RWMutexType::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    if (it != GetDatas().end()) {
        return GetDatas()[name];
    }
    return nullptr;
}

void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
    RWMutexType::ReadLock lock(GetMutex());
    ConfigVarMap& m = GetDatas();
    for (auto& item : m) {
        cb(item.second);
    }
}
}  // namespace sylar