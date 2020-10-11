/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-06-05 17:11:30
 * @LastEditors: zsj
 * @LastEditTime: 2020-07-05 21:10:28
 */
#include "util.h"

#include <dirent.h>
#include <execinfo.h>
#include <limits.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <limits>

#include "fiber.h"
#include "log.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

pid_t GetThreadId() { return syscall(SYS_gettid); }

uint32_t GetFiberId() { return sylar::Fiber::GetFiberId(); }


void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    void** array = (void**)malloc((sizeof(void*)) * size);
    size_t s = ::backtrace(array, size);
    char** strings = backtrace_symbols(array, s);
    if (strings == NULL) {
        SYLAR_LOG_ERROR(g_logger) << "backtrace_symbols error";
        return;
    }

    for (size_t i = skip; i < s; i++) {
        bt.push_back(strings[i]);
    }
    free(strings);
    free(array);
}
std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for (size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}


uint64_t GetCurrentMS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}
uint64_t GetCurrentUS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}


std::string TimeToString(time_t ts, const std::string& format) {
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

void FSUtil::listAllFile(std::vector<std::string>& files,
                         const std::string& path, const std::string& subfix) {
    if (access(path.c_str(), 0) != 0) {
        return;
    }
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }

    struct dirent* dp = nullptr;
    while ((dp = readdir(dir)) != nullptr) {
        if (dp->d_type == DT_DIR) {
            if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
                continue;
            }
            listAllFile(files, path + "/" + dp->d_name, subfix);
        } else if (dp->d_type == DT_REG) {
            std::string filename(dp->d_name);
            if (subfix.empty()) {
                files.push_back(path + "/" + filename);
            } else {
                if (filename.size() < subfix.size()) {
                    continue;
                }
                if (filename.substr(filename.length() - subfix.length()) ==
                    subfix) {
                    files.push_back(path + "/" + filename);
                }
            }
        }
    }
    closedir(dir);
    // for(auto & iter : files){
    //     SYLAR_LOG_DEBUG(g_logger) << iter;
    // }
}

static int __lstat(const char* file, struct stat* st = nullptr) {
    struct stat lst;

    /**
     * @brief 用来获取linux操作系统下文件的属性
     * lstat是stat函数的一个宏定义，底层使用的是stat函数
     * int stat (const char *__restrict __file,struct stat *__restrict __buf)
     * @param[in] 文件路径
     * @param[out] 获取的文件信息
     * @return 0,成功  -1，失败
     */
    int ret = lstat(file, &lst);
    if (st) {
        *st = lst;
    }
    return ret;
}

static int __mkdir(const char* dirname) {
    if (access(dirname, F_OK) == 0) {
        return 0;
    }
    return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

bool FSUtil::Mkdir(const std::string& dirname) {
    //如果当前目录存在  直接返回
    if (__lstat(dirname.c_str()) == 0) {
        return true;
    }

    /**
     * @brief 创建一个传入字符串的副本，该副本是用malloc申请的内存，需要free释放
     * char *strdup (const char *__s)
     * @param[in] __s  传入字符串
     * @return 成功返回一个字符串指针，失败返回NULL
     */
    char* path = strdup(dirname.c_str());

    /**
     * @brief 查找给定字符串中某一个特定字符
     * strchr() 函数会依次检索字符串 str 中的每一个字符，直到遇见字符 c，
     *      或者到达字符串末尾（遇见\0）。
     * const char * strchr (const char *__s, int __c)
     * @param[in] __s 字符串
     * @param[in] __c 需要查找的字符串
     * @return 返回在字符串__s中第一次出现字符c的位置，如果未找到则返回NULL
     */
    char* ptr = strchr(path + 1, '/');
    do {
        for (; ptr; (*ptr = '/', ptr = strchr(ptr + 1, '/'))) {
            *ptr = '\0';
            if (__mkdir(path) != 0) {
                break;
            }
        }
        //出错
        if (ptr != nullptr) {
            break;
        } else if (__mkdir(path) != 0) {
            break;
        }
        free(path);
        SYLAR_LOG_DEBUG(g_logger) << "success mkdir " << path;
        return true;
    } while (0);
    free(path);
    return false;
}
bool FSUtil::IsRunningPidfile(const std::string& pidfile) {
    //判断文件是否存在
    if (__lstat(pidfile.c_str()) != 0) {
        return false;
    }

    std::ifstream ifs(pidfile);
    std::string line;
    if (!ifs || !std::getline(ifs, line)) {
        return false;
    }

    if (line.empty()) {
        return false;
    }
    pid_t pid = atoi(line.c_str());
    if (pid <= 1) {
        return false;
    }

    /**
     * @brief 用于向任何进程组或进程发送信号
     * int kill (__pid_t __pid, int __sig)
     * @param __pid 可能有以下四种
     *      __pid > 0,pid是信号欲送往的进程的标识。
     *      __pid ==
     * 0,信号将送往所有与调用kill()的那个进程属同一个使用组的进程。
     *      __pid ==
     * -1,信号将送往所有调用进程有权给其发送信号的进程，除了进程1(init)。
     *      __pid < -1,信号将送往以-pid为组标识的进程。
     * @param __sig 准备发送的信号代码
     *      ! 假如其值为零则没有任何信号送出，但是系统会执行错误检查，
     *      ! 通常会利用sig值为零来检验某个进程是否仍在执行。
     * @return 0,成功   -1，失败
     */
    if (kill(pid, 0) != 0) {  //检测pid进程是否还在执行
        return false;
    }
    return true;
}


bool FSUtil::Unlink(const std::string& filename, bool exist) {
    if (!exist && __lstat(filename.c_str())) {
        return true;
    }

    return ::unlink(filename.c_str()) == 0;
}

bool FSUtil::Rm(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st)) {
        return true;
    }
    if (!(st.st_mode & S_IFDIR)) {
        return Unlink(path);
    }

    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }

    bool ret = true;
    struct dirent* dp = nullptr;
    while ((dp = readdir(dir))) {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
            continue;
        }

        std::string dirname = path + "/" + dp->d_name;
        ret = Rm(dirname);
    }
    return ret;
}


bool FSUtil::Mv(const std::string& from, const std::string& to) {
    if (!Rm(to)) {
        return false;
    }
    return rename(from.c_str(), to.c_str()) == 0;
}


bool FSUtil::Realpath(const std::string& path, std::string& rpath) {
    if (__lstat(path.c_str())) {
        return false;
    }

    char* ptr = ::realpath(path.c_str(), nullptr);
    if (nullptr == ptr) {
        return false;
    }

    std::string(ptr).swap(rpath);
    free(ptr);
    return true;
}
bool FSUtil::Symlink(const std::string& from, const std::string& to) {
    if (!Rm(to)) {
        return false;
    }

    return ::symlink(from.c_str(), to.c_str()) == 0;
}


std::string FSUtil::Dirname(const std::string& filename) {
    if (filename.empty()) {
        return ".";
    }

    auto pos = filename.rfind('/');
    if (pos == 0) {
        return "/";
    } else if (pos == std::string::npos) {
        return ".";
    } else {
        return filename.substr(0, pos);
    }
}
std::string FSUtil::Basename(const std::string& filename) {
    if (filename.empty()) {
        return filename;
    }

    auto pos = filename.rfind('/');
    if (pos == std::string::npos) {
        return filename;
    } else {
        return filename.substr(pos + 1);
    }
}
bool FSUtil::OpenForRead(std::ifstream& ifs, const std::string& filename,
                         std::ios_base::openmode mode) {
    ifs.open(filename.c_str(), mode);
    return ifs.is_open();
}
bool FSUtil::OpenForWrite(std::ofstream& ofs, const std::string& filename,
                          std::ios_base::openmode mode) {
    ofs.open(filename.c_str(), mode);
    if (!ofs.is_open()) {
        std::string dir = Dirname(filename);
        Mkdir(dir);
        ofs.open(filename.c_str(), mode);
    }
    return ofs.is_open();
}


int8_t TypeUtil::ToChar(const std::string& str) {
    if (str.empty()) {
        return 0;
    }
    return *str.begin();
}
int64_t TypeUtil::Atoi(const std::string& str) {
    if (str.empty()) {
        return 0;
    }
    return strtoull(str.c_str(), nullptr, 10);
}
double TypeUtil::Atof(const std::string& str) {
    if (str.empty()) {
        return 0;
    }
    return atof(str.c_str());
}
int8_t TypeUtil::ToChar(const char* str) {
    if (str == nullptr) {
        return 0;
    }
    return *str;
}
int64_t TypeUtil::Atoi(const char* str) {
    if (str == nullptr) {
        return 0;
    }
    return strtoull(str, nullptr, 10);
}
double TypeUtil::Atof(const char* str) {
    if (str == nullptr) {
        return 0;
    }

    return atof(str);
}


}  // namespace sylar