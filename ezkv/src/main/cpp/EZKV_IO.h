//
// Created by chidh on 2021/3/25.
//

#ifndef EZKV_EZKV_IO_H
#define EZKV_EZKV_IO_H
#ifdef __cplusplus

#include "EZKV.h"
#include "EZKVPredef.h"

std::string mmapedKVKey(const std::string &mmapID);
EZKVPath_t mappedKVPathWithID(const std::string &mmapID);

template <typename T>
void clearDictionary(T *dic) {
    if (!dic) {
        return;
    }
    dic->clear();
}

#endif
#endif //EZKV_EZKV_IO_H
