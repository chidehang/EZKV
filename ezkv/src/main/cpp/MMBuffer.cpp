//
// Created by chidh on 2021/3/26.
//

#include <cerrno>
#include "MMBuffer.h"

using namespace std;

namespace ezkv {

    MMBuffer::MMBuffer(size_t length) {
        if (length > SmallBufferSize()) {
            type = MMBufferType_Normal;
            isNoCopy = MMBufferCopy;
            size = length;
            ptr = malloc(size);
            if (!ptr) {
                throw std::runtime_error(strerror(errno));
            }
        } else {
            type = MMBufferType_Small;
            paddedSize = static_cast<uint8_t>(length);
        }
    }

    MMBuffer::MMBuffer(void *source, size_t length, MMBufferCopyFlag flag) : isNoCopy(flag) {
        if (isNoCopy == MMBufferCopy) {
            if (length > SmallBufferSize()) {
                type = MMBufferType_Normal;
                size = length;
                ptr = malloc(size);
                if (!ptr) {
                    throw std::runtime_error(strerror(errno));
                }
                memcpy(ptr, source, size);
            } else {
                type = MMBufferType_Small;
                paddedSize = static_cast<uint8_t>(length);
                memcpy(paddedBuffer, source, length);
            }
        } else {
            type = MMBufferType_Normal;
            size = length;
            ptr = source;
        }
    }

    MMBuffer::MMBuffer(MMBuffer &&other) noexcept : type(other.type) {
        if (type == MMBufferType_Normal) {
            size = other.size;
            ptr = other.ptr;
            isNoCopy = other.isNoCopy;
            other.detach();
        } else {
            paddedSize = other.paddedSize;
            memcpy(paddedBuffer, other.paddedBuffer, paddedSize);
        }
    }

    MMBuffer &MMBuffer::operator=(MMBuffer &&other) noexcept {
        if (type == MMBufferType_Normal) {
            if (other.type == MMBufferType_Normal) {
                std::swap(isNoCopy, other.isNoCopy);
                std::swap(size, other.size);
                std::swap(ptr, other.ptr);
            } else {
                type = MMBufferType_Small;
                if (isNoCopy == MMBufferNoCopy) {
                    if (ptr) {
                        free(ptr);
                    }
                }
                paddedSize = other.paddedSize;
                memcpy(paddedBuffer, other.paddedBuffer, paddedSize);
            }
        } else {
            if (other.type == MMBufferType_Normal) {
                type = MMBufferType_Normal;
                isNoCopy = other.isNoCopy;
                size = other.size;
                ptr = other.ptr;
                other.detach();
            } else {
                uint8_t tmp[SmallBufferSize()];
                memcpy(tmp, other.paddedBuffer, other.paddedSize);
                memcpy(other.paddedBuffer, paddedBuffer, paddedSize);
                memcpy(paddedBuffer, tmp, other.paddedSize);
                std::swap(paddedSize, other.paddedSize);
            }
        }
        return *this;
    }

    MMBuffer::~MMBuffer() {
        if (isStoredOnStack()) {
            return;
        }

        if (isNoCopy == MMBufferCopy && ptr) {
            free(ptr);
        }
    }

    void MMBuffer::detach() {
        auto memsetPtr = (size_t *) &type;
        *memsetPtr = 0;
    }
}


