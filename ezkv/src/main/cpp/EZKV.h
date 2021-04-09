//
// Created by chidh on 2021/3/24.
//

#ifndef EZKV_EZKV_H
#define EZKV_EZKV_H
#ifdef __cplusplus

#include "EZKVPredef.h"
#include "MMBuffer.h"
#include "KeyValueHolder.h"

namespace ezkv {
class CodedOutputData;
class MemoryFile;
} // namespace ezkv

class EZKV {
    EZKV(const std::string &mmapID);

    std::string m_mmapID;
    EZKVPath_t m_path;
    ezkv::EZKVMAP *m_dic;

    ezkv::MemoryFile *m_file;
    size_t m_actualSize;
    ezkv::CodedOutputData *m_output;

    using EZKVKey_t = const std::string &;
    static bool isKeyEmpty(EZKVKey_t key) { return key.empty(); }

    void loadFromFile();

    void checkDataValid();

    bool isFileValid();

    size_t readActualSize();

    bool writeActualSize(size_t size);

    void oldStyleWriteActualSize(size_t actualSize);

    bool ensureMemorySize(size_t newSize);

    bool doFullWriteBack(std::pair<ezkv::MMBuffer, size_t> prepareData);

    // isDataHolder: avoid memory copying
    bool setDataForKey(ezkv::MMBuffer &&data, EZKVKey_t key, bool isDataHolder = false);

    ezkv::MMBuffer getDataForKey(EZKVKey_t key);

    bool removeDataForKey(EZKVKey_t key);

    using KVHolderRet_t = std::pair<bool, ezkv::KeyValueHolder>;
    // isDataHolder: avoid memory copying
    KVHolderRet_t doAppendDataWithKey(const ezkv::MMBuffer &data, const ezkv::MMBuffer &key, bool isDataHolder, uint32_t keyLength);
    KVHolderRet_t appendDataWithKey(const ezkv::MMBuffer &data, EZKVKey_t key, bool isDataHolder = false);
    KVHolderRet_t appendDataWithKey(const ezkv::MMBuffer &data, const ezkv::KeyValueHolder &kvHolder, bool isDataHolder = false);

public:
    // 全局初始化
    static void initializeEZKV(const EZKVPath_t &rootDir);

    // 获取默认EZKV实例
    static EZKV *defaultEZKV();

    static EZKV *ezkvWithID(const std::string &ezkvID,
                            int size = ezkv::DEFAULT_EZKV_SIZE);

    bool set(int32_t value, EZKVKey_t key);

    bool set(const std::string &value, EZKVKey_t key);

    bool getString(EZKVKey_t key, std::string &result);

    int32_t getInt32(EZKVKey_t key, int32_t defaultValue = 0);

    void removeValueForKey(EZKVKey_t key);

    size_t totalSize();

    size_t count();

    // you don't need to call this, really, I mean it
    // unless you worry about running out of battery
    void sync(SyncFlag flag = EZKV_SYNC);
};

#endif
#endif //EZKV_EZKV_H
