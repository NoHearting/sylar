/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-10-10 17:36:10
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-11 15:38:39
 */
#include <iostream>

#include "sylar/ds/lru_cache.h"

void test_lru() {
    sylar::ds::LruCache<int, int> cache(30, 10);
    cache.setPruneCallback([](const int& key, const int& value) {
        std::cout << "exec callback:"
                  << " key=" << key << " value=" << value << std::endl;
    });
    for (int i = 0; i < 105; i++) {
        cache.set(i, i * 30);
    }

    for (int i = 0; i < 105; i++) {
        int v;
        if (cache.get(i, v)) {
            std::cout << "get:" << i << " - " << v << std::endl;
        }
    }

    std::cout << cache.toStatusString() << std::endl;
}


void test_hash_lru() {
    sylar::ds::HashLruCache<int, int> cache(2, 30, 10);
    cache.setPruneCallback([](const int& key, const int& value) {
        std::cout << "exec callback:"
                  << " key=" << key << " value=" << value << std::endl;
    });
    for (int i = 0; i < 105; i++) {
        cache.set(i, i * 100);
    }
    for (int i = 0; i < 105; i++) {
        int v;
        if (cache.get(i, v)) {
            std::cout << "get:" << i << " - " << v << std::endl;
        }
    }
    std::cout << cache.toStatusString() << std::endl;
}

int main(int argc, char** argv) {
    test_lru();
    test_hash_lru();
}