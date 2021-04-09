//
// Created by chidh on 2021/3/29.
//

#include "CodedInputData.h"
#include "KeyValueHolder.h"

using namespace std;

namespace ezkv {

    CodedInputData::CodedInputData(const void *oData, size_t length)
            : m_ptr((uint8_t *) oData), m_size(length), m_position(0) {
    }

    void CodedInputData::seek(size_t addedSize) {
        if (m_position + addedSize > m_size) {
            throw out_of_range("OutOfSpace");
        }
        m_position += addedSize;
    }

    int32_t CodedInputData::readInt32() {
        return this->readRawVarint32();
    }

    int32_t CodedInputData::readRawVarint32() {
        int8_t tmp = this->readRawByte();
        if (tmp >= 0) {
            // 0xxxxxxx
            return tmp;
        }
        // 1xxxxxxx
        int32_t result = tmp & 0x7f;
        if ((tmp = this->readRawByte()) >= 0) {
            // 0yyyyyyy => 00yyyyyy y0000000 | 1xxxxxxx => 00yyyyyy yxxxxxxx
            result |= tmp << 7;
        } else {
            // 1yyyyyyy => 0yyyyyyy => 00yyyyyy y0000000 | 1xxxxxxx => 00yyyyyy yxxxxxxx
            result |= (tmp & 0x7f) << 7;
            if ((tmp = this->readRawByte()) >= 0) {
                // 0zzzzzzz => 000zzzzz zz000000 00000000 | 00yyyyyy yxxxxxxx = 000zzzzz zzyyyyyy yxxxxxxx
                result |= tmp << 14;
            } else {
                // 1zzzzzzz => 0zzzzzzz => ··· => 000zzzzz zzyyyyyy yxxxxxxx
                result |= (tmp & 0x7f) << 14;
                if ((tmp = this->readRawByte()) >= 0) {
                    result |= tmp << 21;
                } else {
                    result |= (tmp & 0x7f) << 21;
                    result |= (tmp = this->readRawByte()) << 28;
                    if (tmp < 0) {
                        // discard upper 32 bits
                        for (int i = 0; i < 5; i++) {
                            if (this->readRawByte() >= 0) {
                                return result;
                            }
                        }
                        throw invalid_argument("InvalidProtocolBuffer malformed varint32");
                    }
                }
            }
        }
        return result;
    }

    int8_t CodedInputData::readRawByte() {
        if (m_position == m_size) {
            auto msg = "reach end, m_position: " + to_string(m_position) + ", m_size: " + to_string(m_size);
            throw out_of_range(msg);
        }
        auto *bytes = (int8_t *) m_ptr;
        return bytes[m_position++];
    }

    std::string CodedInputData::readString(KeyValueHolder &kvHolder) {
        kvHolder.offset = static_cast<uint32_t>(m_position);

        // 读key长度
        int32_t size = this->readRawVarint32();
        if (size < 0) {
            throw length_error("InvalidProtocolBuffer negativeSize");
        }

        auto s_size = static_cast<size_t>(size);
        if (s_size <= m_size - m_position) {
            kvHolder.keySize = static_cast<uint16_t>(s_size);
            string result((char *) (m_ptr + m_position), s_size);
            m_position += s_size;
            return result;
        } else {
            throw out_of_range("readString(KeyValueHolder): InvalidProtocolBuffer truncatedMessage");
        }
    }

    void CodedInputData::readData(KeyValueHolder &kvHolder) {
        // 读value长度
        int32_t size = this->readRawVarint32();
        if (size < 0) {
            throw length_error("InvalidProtocolBuffer negativeSize");
        }

        auto s_size = static_cast<size_t>(size);
        if (s_size <= m_size - m_position) {
            kvHolder.computedKVSize = static_cast<uint16_t>(m_position - kvHolder.offset);
            kvHolder.valueSize = static_cast<uint32_t>(s_size);

            m_position += s_size;
        } else {
            throw out_of_range("readData(): InvalidProtocolBuffer truncatedMessage");
        }
    }

    std::string CodedInputData::readString() {
        // 存string时，value项多写入了valueLength，这里取出用到
        int32_t size = readRawVarint32();
        if (size < 0) {
            throw length_error("InvalidProtocolBuffer negativeSize");
        }

        auto s_size = static_cast<size_t>(size);
        // s_size即string.length
        if (s_size <= m_size - m_position) {
            string result((char *) (m_ptr + m_position), s_size);
            m_position += s_size;
            return result;
        } else {
            throw out_of_range("readString(): InvalidProtocolBuffer truncatedMessage");
        }
    }
} // namespace ezkv
