//
// Created by chidh on 2021/3/25.
//

#ifndef EZKV_PBUTILITY_H
#define EZKV_PBUTILITY_H
#ifdef __cplusplus

#include "EZKVPredef.h"

namespace ezkv {

    // 利用联合体的特性实现类型转换
    template <typename T, typename P>
    union Converter {
        static_assert(sizeof(T) == sizeof(P), "size not match");
        T first;
        P second;
    };

    static inline uint32_t Int32ToUInt32(int32_t v) {
        Converter<int32_t, uint32_t> converter;
        converter.first = v;
        return converter.second;
    }

    static inline int32_t UInt32ToInt32(uint32_t v) {
        Converter<int32_t, uint32_t> converter;
        converter.second = v;
        return converter.first;
    }

    static inline uint64_t Int64ToUInt64(int64_t value) {
        Converter<int64_t, uint64_t> converter;
        converter.first = value;
        return converter.second;
    }

    static inline int64_t UInt64ToInt64(uint64_t value) {
        Converter<int64_t, uint64_t> converter;
        converter.second = value;
        return converter.first;
    }

    static inline int32_t logicalRightShift32(int32_t value, uint32_t spaces) {
        return UInt32ToInt32((Int32ToUInt32(value) >> spaces));
    }

    static inline int64_t logicalRightShift64(int64_t value, uint32_t spaces) {
        return UInt64ToInt64(Int64ToUInt64(value) >> spaces);
    }

    constexpr uint32_t LittleEdian32Size = 4;

    constexpr uint32_t pbFixed32Size() {
        return LittleEdian32Size;
    }

    extern uint32_t pbRawVarint32Size(uint32_t value);

    static inline uint32_t pbRawVarint32Size(int32_t value) {
        return pbRawVarint32Size(Int32ToUInt32(value));
    }

    static inline uint32_t pbInt32Size(int32_t value) {
        if (value >= 0) {
            return pbRawVarint32Size(value);
        } else {
            // 负数
            return 10;
        }
    }

} // namespace ezkv

#endif
#endif //EZKV_PBUTILITY_H
