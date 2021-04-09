//
// Created by chidh on 2021/3/30.
//

#ifndef EZKV_KEYVALUEHOLDER_H
#define EZKV_KEYVALUEHOLDER_H
#ifdef __cplusplus

#include "MMBuffer.h"

namespace ezkv {
#pragma pack(push, 1)

    struct KeyValueHolder {

        uint16_t computedKVSize; // internal use only（基于offset的偏移量 = key size + key + value size， 可以快速定位到该KV对的value值）
        uint16_t keySize;
        uint32_t valueSize;
        uint32_t offset; // 从ptr开始到该KV对的偏移量，可以快速定位到该KV对

        KeyValueHolder() = default;
        KeyValueHolder(uint32_t keyLength, uint32_t valueLength, uint32_t offset);

        MMBuffer toMMBuffer(const void *basePtr) const;
    };

#pragma pack(pop)
} // namespace ezkv

#endif
#endif //EZKV_KEYVALUEHOLDER_H
