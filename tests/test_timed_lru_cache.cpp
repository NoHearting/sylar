/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-10-11 10:53:23
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-12 17:04:48
 */
#include "sylar/ds/timed_lru_cache.h"
#include "sylar/log.h"


static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_timed_lru_cache() {
    sylar::ds::TimedLruCache<int, int> cache(30, 10);
    for (int i = 0; i < 105; i++) {
        cache.set(i, i * 100, 1000);
    }
    for (int i = 0; i < 105; i++) {
        int v;
        if (cache.get(i, v)) {
            SYLAR_LOG_INFO(g_logger)
                << "get: " << i << " - " << v << " - " << cache.get(i);
        }
    }

    cache.set(1000, 11, 1000 * 10);
    SYLAR_LOG_INFO(g_logger) << cache.toStatusString();
    sleep(2);
    SYLAR_LOG_INFO(g_logger) << "check_timeout:" << cache.checkTimeout();
    SYLAR_LOG_INFO(g_logger) << cache.toStatusString();
}

void test_hash_timed_lru_cache() {
    sylar::ds::HashTimedLruCache<int, int> cache(2, 30, 10);
    for (int i = 0; i < 105; i++) {
        cache.set(i, i * 100, 1000);
    }

    for (int i = 0; i < 105; i++) {
        int v;
        if (cache.get(i, v)) {
            SYLAR_LOG_INFO(g_logger)
                << "get: " << i << " - " << v << " - " << cache.get(i);
        }
    }

    cache.set(1000, 11, 1000 * 10);
    SYLAR_LOG_INFO(g_logger) << cache.toStatusString();
    sleep(2);
    SYLAR_LOG_INFO(g_logger) << "check_timeout:" << cache.checkTimeout();
    SYLAR_LOG_INFO(g_logger) << cache.toStatusString();
}

int main(int argc, char** argv) {
    test_timed_lru_cache();
    test_hash_timed_lru_cache();
    return 0;
}