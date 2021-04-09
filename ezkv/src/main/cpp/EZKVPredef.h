//
// Created by chidh on 2021/3/24.
//

#ifndef EZKV_EZKVPREDEF_H
#define EZKV_EZKVPREDEF_H
#ifdef __cplusplus

#include <string>
#include <unordered_map>

constexpr auto EZKV_PATH_SLASH = "/";

using EZKVFileHandle_t = int;
using EZKVPath_t = std::string;

enum SyncFlag : bool {EZKV_SYNC = true, EZKV_ASYNC = false};

namespace ezkv {

    extern size_t DEFAULT_EZKV_SIZE;

#define DEFAULT_EZKV_ID "ezkv.default"

    class MMBuffer;
    struct KeyValueHolder;

    using EZKVMAP = std::unordered_map<std::string, KeyValueHolder>;
}

#endif
#endif //EZKV_EZKVPREDEF_H
