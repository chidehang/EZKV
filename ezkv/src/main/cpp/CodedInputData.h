//
// Created by chidh on 2021/3/29.
//

#ifndef EZKV_CODEDINPUTDATA_H
#define EZKV_CODEDINPUTDATA_H
#ifdef __cplusplus

#include "EZKVPredef.h"

namespace ezkv {

    class CodedInputData {
        uint8_t *const m_ptr;
        size_t m_size;
        size_t m_position;

        int8_t readRawByte();

        int32_t readRawVarint32();

    public:
        CodedInputData(const void *oData, size_t length);

        bool isAtEnd() const {return m_position == m_size;};

        void seek(size_t addedSize);

        int32_t readInt32();

        void readData(KeyValueHolder &kvHolder);

        std::string readString();
        std::string readString(KeyValueHolder &kvHolder);
    };
} // namespace ezkv

#endif
#endif //EZKV_CODEDINPUTDATA_H
