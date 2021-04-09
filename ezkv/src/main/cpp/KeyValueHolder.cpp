//
// Created by chidh on 2021/3/30.
//

#include "KeyValueHolder.h"
#include "PBUtility.h"

namespace ezkv {

    KeyValueHolder::KeyValueHolder(uint32_t keyLength, uint32_t valueLength, uint32_t off)
            : keySize(static_cast<uint16_t>(keyLength)), valueSize(valueLength), offset(off) {
        computedKVSize = keySize + static_cast<uint16_t>(pbRawVarint32Size(keySize));
        computedKVSize += static_cast<uint16_t>(pbRawVarint32Size(valueSize));
    }

    MMBuffer KeyValueHolder::toMMBuffer(const void *basePtr) const {
        auto realPtr = (uint8_t *) basePtr + offset;
        realPtr += computedKVSize;
        return MMBuffer(realPtr, valueSize, MMBufferNoCopy);
    }
} // namespace ezkv

