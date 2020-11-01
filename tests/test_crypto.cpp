#include <assert.h>
#include <openssl/ssl.h>

#include <iostream>

#include "sylar/util.h"


void test_aes256ecb() {
    for (int i = 0; i < 4097; i++) {
        auto str = sylar::random_string(i);
        std::string encode;
        encode.resize(i + 30);
        std::string decode;
        decode.resize(i + 30);
        auto key = sylar::random_string(32);
        int32_t len = sylar::CryptoUtil::AES256Ecb(
            key.c_str(), str.c_str(), str.size(), &encode[0], true);
        std::cout << "encode_len:" << len << std::endl;
        if (len > 0) {
            encode.resize(len);
        }
        len = sylar::CryptoUtil::AES256Ecb(key.c_str(), encode.c_str(),
                                           encode.size(), &decode[0], false);
        std::cout << "decode_len:" << len << std::endl;
        if (len > 0) {
            decode.resize(true);
        }
        std::cout << (str == decode) << std::endl;
        // assert(str == decode);
    }
}

int main(int argc, char** argv) {
    test_aes256ecb();
    return 0;
}