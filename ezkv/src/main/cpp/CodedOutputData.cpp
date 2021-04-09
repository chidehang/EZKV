//
// Created by chidh on 2021/3/26.
//

#include "CodedOutputData.h"
#include "PBUtility.h"
#include <stdexcept>
#include <string>

using namespace std;

namespace ezkv {

    CodedOutputData::CodedOutputData(void *ptr, size_t len)
            : m_ptr((uint8_t *) ptr), m_size(len), m_position(0) {
    }

    void CodedOutputData::seek(size_t addedSize) {
        m_position += addedSize;
        if (m_position > m_size) {
            throw out_of_range("OutOfSpace");
        }
    }

    void CodedOutputData::writeInt32(int32_t value) {
        if (value >= 0) {
            this->writeRawVarint32(value);
        } else {
            this->writeRawVarint64(value);
        }
    }

    void CodedOutputData::writeRawVarint32(int32_t value) {
        // 按变长编码方式写入
        while (true) {
            if ((value & ~0x7f) == 0) {
                this->writeRawByte(static_cast<uint8_t>(value));
                return;
            } else {
                this->writeRawByte(static_cast<uint8_t>((value & 0x7F) | 0x80));
                value = logicalRightShift32(value, 7);
            }
        }
    }

    void CodedOutputData::writeRawVarint64(int64_t value) {
        // 按变长编码方式写入
        while (true) {
            if ((value & ~0x7f) == 0) {
                this->writeRawByte(static_cast<uint8_t>(value));
                return;
            } else {
                this->writeRawByte(static_cast<uint8_t>((value & 0x7f) | 0x80));
                value = logicalRightShift64(value, 7);
            }
        }
    }

    void CodedOutputData::writeRawByte(uint8_t value) {
        if (m_position == m_size) {
            throw out_of_range("m_position: " + to_string(m_position) + " m_size: " + to_string(m_size));
        }

        m_ptr[m_position++] = value;
    }

    size_t CodedOutputData::spaceLeft() {
        if (m_size <= m_position) {
            return 0;
        }
        return m_size - m_position;
    }

    uint8_t *CodedOutputData::curWritePointer() {
        return m_ptr + m_position;
    }

    void CodedOutputData::writeData(const MMBuffer &value) {
        this->writeRawVarint32((int32_t) value.length());
        this->writeRawData(value);
    }

    void CodedOutputData::writeRawData(const MMBuffer &data) {
        size_t numberOfBytes = data.length();
        if (m_position + numberOfBytes > m_size) {
            auto msg = "m_position: " + to_string(m_position) + ", numberOfBytes: " + to_string(numberOfBytes) +
                       ", m_size: " + to_string(m_size);
            throw out_of_range(msg);
        }
        memcpy(m_ptr + m_position, data.getPtr(), numberOfBytes);
        m_position += numberOfBytes;
    }
} // namespace ezkv

