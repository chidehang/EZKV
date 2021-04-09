//
// Created by chidh on 2021/3/25.
//

#include <pthread.h>
#include <unordered_map>
#include "EZKV.h"
#include "MemoryFile.h"
#include "EZKVLog.h"
#include "EZKV_IO.h"
#include "PBUtility.h"
#include "MMBuffer.h"
#include "CodedOutputData.h"
#include "CodedInputData.h"

using namespace std;
using namespace ezkv;

unordered_map<std::string, EZKV *> *g_instanceDic;
EZKVPath_t g_rootDir;
size_t ezkv::DEFAULT_EZKV_SIZE;

void initialize() {
    g_instanceDic = new unordered_map<std::string, EZKV *>;
    ezkv::DEFAULT_EZKV_SIZE = ezkv::getPageSize();
}

pthread_once_t once_control = PTHREAD_ONCE_INIT;

void EZKV::initializeEZKV(const EZKVPath_t &rootDir) {
    pthread_once(&once_control, initialize);

    g_rootDir = rootDir;
    mkPath(g_rootDir);
    LOGI("root dir: %s", g_rootDir.c_str());
}

EZKV *EZKV::defaultEZKV() {
    return ezkvWithID(DEFAULT_EZKV_ID, DEFAULT_EZKV_SIZE);
}

EZKV *EZKV::ezkvWithID(const string &ezkvID, int size) {
    if (ezkvID.empty()) {
        return nullptr;
    }

    auto mmapKey = mmapedKVKey(ezkvID);
    // 从map查找缓存
    auto itr = g_instanceDic->find(mmapKey);
    if (itr != g_instanceDic->end()) {
        EZKV *kv = itr->second;
        return kv;
    }

    auto kv = new EZKV(mmapKey);
    (*g_instanceDic)[mmapKey] = kv;
    return kv;
}

static EZKVPath_t encodeFilePath(const string &mmapID) {
    return mmapID;
}

std::string mmapedKVKey(const std::string &mmapID) {
    return mmapID;
}

EZKVPath_t mappedKVPathWithID(const std::string &mmapID) {
    return g_rootDir + EZKV_PATH_SLASH + encodeFilePath(mmapID);
}

bool EZKV::isFileValid() {
    return m_file->isFileValid();
}

void EZKV::sync(SyncFlag flag) {
    if (!m_file->isFileValid()) {
        return;
    }

    m_file->msync(flag);
}

bool EZKV::set(int32_t value, EZKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    // 计算按照变长编码方式存储value需要的大小
    size_t size = pbInt32Size(value);
    MMBuffer data(size);
    CodedOutputData output(data.getPtr(), size);
    // 写入value到MMBuffer中创建的空间
    output.writeInt32(value);

    return setDataForKey(move(data), key);
}

bool EZKV::set(const string &value, EZKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }
    return setDataForKey(MMBuffer((void *) value.data(), value.length(), MMBufferNoCopy), key, true);
}

void EZKV::removeValueForKey(EZKVKey_t key) {
    if (isKeyEmpty(key)) {
        return;
    }
    removeDataForKey(key);
}

int32_t EZKV::getInt32(EZKVKey_t key, int32_t defaultValue) {
    if (isKeyEmpty(key)) {
        return defaultValue;
    }

    auto data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            return input.readInt32();
        } catch (std::exception &e) {
            LOGE("%s", e.what());
        }
    }
    return defaultValue;
}

bool EZKV::getString(EZKVKey_t key, string &result) {
    if (isKeyEmpty(key)) {
        return false;
    }

    auto data = getDataForKey(key);
    if (data.length() > 0) {
        try {
            CodedInputData input(data.getPtr(), data.length());
            result = input.readString();
            return true;
        } catch (std::exception &e) {
            LOGE("%s", e.what());
        }
    }
    return false;
}

size_t EZKV::totalSize() {
    return m_file->getFileSize();
}

size_t EZKV::count() {
    return m_dic->size();
}

