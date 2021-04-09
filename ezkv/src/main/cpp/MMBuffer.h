//
// Created by chidh on 2021/3/26.
//

#ifndef EZKV_MMBUFFER_H
#define EZKV_MMBUFFER_H
#ifdef __cplusplus

#include "EZKVPredef.h"

namespace ezkv {

    enum MMBufferCopyFlag : bool {
        MMBufferCopy = false,
        MMBufferNoCopy = true,
    };

#pragma pack(push, 1)

    class MMBuffer {
        enum MMBufferType : uint8_t {
            MMBufferType_Small,  // store small buffer in stack memory
            MMBufferType_Normal, // store in heap memory
        };
        MMBufferType type;

        union {
            struct {
                MMBufferCopyFlag isNoCopy;
                size_t size;
                void *ptr;
            };
            struct {
                uint8_t paddedSize;
                // make at least 10 bytes to hold all primitive types (negative int32, int64, double etc) on 32 bit device
                // on 64 bit device it's guaranteed larger than 10 bytes
                uint8_t paddedBuffer[10]; // 用于存储较小的基本类型的值，存储在栈上，而不是堆上，避免malloc和free
            };
        };

        static constexpr size_t SmallBufferSize() {
            return sizeof(MMBuffer) - offsetof(MMBuffer, paddedBuffer);
        }

    public:
        explicit MMBuffer(size_t length = 0);
        MMBuffer(void *source, size_t length, MMBufferCopyFlag flag = MMBufferCopy);

        MMBuffer(MMBuffer &&other) noexcept;
        MMBuffer &operator=(MMBuffer &&other) noexcept;

        ~MMBuffer();

        bool isStoredOnStack() const {return (type == MMBufferType_Small);}

        void *getPtr() const {return isStoredOnStack()? (void *) paddedBuffer : ptr; }

        size_t length() const {return isStoredOnStack()? paddedSize : size;}

        // transfer ownership to others
        void detach();

        // those are expensive, just forbid it for possibly misuse
        explicit MMBuffer(const MMBuffer &other) = delete;
        MMBuffer &operator=(const MMBuffer &other) = delete;
    };

#pragma pack(pop)
}

#endif
#endif //EZKV_MMBUFFER_H
