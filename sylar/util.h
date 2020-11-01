/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-06-05 17:08:39
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-21 20:19:54
 */
#ifndef __SYLAR__UTIL_H__
#define __SYLAR__UTIL_H__

#include <pthread.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <boost/lexical_cast.hpp>
#include <iomanip>
#include <string>
#include <vector>

#include "sylar/util/crypto_util.h"
#include "sylar/util/hash_util.h"
#include "sylar/util/json_util.h"


namespace sylar {

pid_t GetThreadId();
uint32_t GetFiberId();
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2,
                              const std::string& prefix = "");


//时间
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();

std::string TimeToString(time_t ts = time(0),
                         const std::string& format = "%Y-%m-%d %H:%M:%S");


class FSUtil {
public:
    static void listAllFile(std::vector<std::string>& files,
                            const std::string& path, const std::string& subfix);


    static bool Mkdir(const std::string& dirname);
    static bool IsRunningPidfile(const std::string& pidfile);

    static bool Rm(const std::string& path);
    static bool Mv(const std::string& from, const std::string& to);
    static bool Realpath(const std::string& path, std::string& rpath);
    static bool Symlink(const std::string& from, const std::string& to);
    static bool Unlink(const std::string& filename, bool exist = false);
    static std::string Dirname(const std::string& filename);
    static std::string Basename(const std::string& filename);
    static bool OpenForRead(std::ifstream& ifs, const std::string& filename,
                            std::ios_base::openmode mode);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename,
                             std::ios_base::openmode mode);
};

template <class Map, class K, class V>
V GetParamValue(const Map& m, const K& k, const V& def = V()) {
    auto it = m.find(k);
    if (it == m.end()) {
        return def;
    }

    try {
        return boost::lexical_cast<V>(it->second);

    } catch (...) {
    }
    return def;
}

template <class Map, class K, class V>
bool CheckGetParamValue(const Map& m, const K& k, V& v) {
    auto it = m.find(k);
    if (it == m.end()) {
        return false;
    }
    try {
        v = boost::lexical_cast<V>(it->second);
        return true;
    } catch (...) {
    }
    return false;
}

class TypeUtil {
public:
    static int8_t ToChar(const std::string& str);
    static int64_t Atoi(const std::string& str);
    static double Atof(const std::string& str);
    static int8_t ToChar(const char* str);
    static int64_t Atoi(const char* str);
    static double Atof(const char* str);
};


class Atomic {
public:
    template <class T, class S>
    static T addFetch(volatile T& t, S v = 1) {
        return __sync_add_and_fetch(&t, (T)v);
    }

    template <class T, class S>
    static T subFetch(volatile T& t, S v = 1) {
        return __sync_sub_and_fetch(&t, (T)v);
    }

    template <class T, class S>
    static T orFetch(volatile T& t, S v) {
        return __sync_or_and_fetch(&t, (T)v);
    }

    template <class T, class S>
    static T xorFetch(volatile T& t, S v) {
        return __sync_xor_and_fetch(&t, (T)v);
    }

    template <class T, class S>
    static T nandFetch(volatile T& t, S v) {
        return __sync_nand_and_fetch(&t, (T)v);
    }

    template <class T, class S>
    static T fetchAdd(volatile T& t, S v = 1) {
        return __sync_fetch_and_add(&t, (T)v);
    }

    template <class T, class S>
    static T fetchSub(volatile T& t, S v = 1) {
        return __sync_fetch_and_sub(&t, (T)v);
    }

    template <class T, class S>
    static T fetchOr(volatile T& t, S v) {
        return __sync_fetch_and_or(&t, (T)v);
    }

    template <class T, class S>
    static T fetchAnd(volatile T& t, S v) {
        return __sync_fetch_and_and(&t, (T)v);
    }

    template <class T, class S>
    static T fetchXor(volatile T& t, S v) {
        return __sync_fetch_and_xor(&t, (T)v);
    }

    template <class T, class S>
    static T fetchNand(volatile T& t, S v) {
        return __sync_fetch_and_nand(&t, (T)v);
    }

    template <class T, class S>
    static T compareAndSwap(volatile T& t, S old_val, S new_val) {
        return __sync_val_compare_and_swap(&t, (T)old_val, (T)new_val);
    }

    template <class T, class S>
    static bool compareAndSwapBool(volatile T& t, S old_val, S new_val) {
        return __sync_bool_compare_and_swap(&t, (T)old_val, (T)new_val);
    }
};


}  // namespace sylar


#endif
