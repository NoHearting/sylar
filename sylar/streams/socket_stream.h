/*
 * @Descripttion:
 * @version:
 * @Author: zsj
 * @Date: 2020-06-16 16:15:59
 * @LastEditors: zsj
 * @LastEditTime: 2020-10-13 14:35:43
 */
#pragma once
#include <memory>

#include "sylar/iomanager.h"
#include "sylar/mutex.h"
#include "sylar/socket.h"
#include "sylar/stream.h"

namespace sylar {

class SocketStream : public Stream {
public:
    typedef std::shared_ptr<SocketStream> ptr;
    SocketStream(Socket::ptr sock, bool owner = true);
    ~SocketStream();

    virtual int read(void* buffer, size_t length) override;
    virtual int read(ByteArray::ptr ba, size_t length) override;

    virtual int write(const void* buffer, size_t length) override;
    virtual int write(ByteArray::ptr ba, size_t length) override;

    virtual void close() override;

    Socket::ptr getSocket() const { return m_socket; }
    bool isConnected() const;

protected:
    Socket::ptr m_socket;
    bool m_owner;
};

}  // namespace sylar
