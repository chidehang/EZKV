//
// Created by chidh on 2021/3/30.
//

#include "PBUtility.h"

namespace ezkv {

    // 获取value按变长编码方式需要占几个字节
    uint32_t pbRawVarint32Size(uint32_t value) {
        if ((value & (0xffffffff << 7)) == 0) {
            return 1;
        } else if ((value & (0xffffffff << 14)) == 0) {
            return 2;
        } else if ((value & (0xffffffff << 21)) == 0) {
            return 3;
        } else if ((value & (0xffffffff << 28)) == 0) {
            return 4;
        }
        return 5;
    }

} // namespace ezkv