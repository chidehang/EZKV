//
// Created by chidh on 2021/3/26.
//

#ifndef EZKV_CODEDOUTPUTDATA_H
#define EZKV_CODEDOUTPUTDATA_H
#ifdef __cplusplus

#include <stdint.h>
#include "MMBuffer.h"

namespace ezkv {

class CodedOutputData {
    uint8_t *const m_ptr;   // 映射内存起始地址
    size_t m_size;          // 文件可写入空间的大小
    size_t m_position;      // 当前位置

public:
    CodedOutputData(void *ptr, size_t len);

    size_t spaceLeft();

    uint8_t *curWritePointer();

    void seek(size_t addedSize);

    void writeRawByte(uint8_t value);

    void writeRawVarint32(int32_t value);

    void writeRawVarint64(int64_t value);

    void writeInt32(int32_t value);

    void writeData(const MMBuffer &value);

    void writeRawData(const MMBuffer &data);
};

} // namespace ezkv

#endif
#endif //EZKV_CODEDOUTPUTDATA_H
