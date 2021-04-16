//
// Created by chidh on 2021/3/25.
//

#include "EZKV_IO.h"
#include "MemoryFile.h"
#include "EZKVLog.h"
#include "PBUtility.h"
#include "EZKV.h"
#include "CodedOutputData.h"
#include "MMBuffer.h"
#include "MiniPBCoder.h"
#include <vector>

using namespace std;
using namespace ezkv;

constexpr uint32_t Fixed32Size = pbFixed32Size();

void EZKV::loadFromFile() {
    if (!m_file->isFileValid()) {
        m_file->reloadFromFile();
    }
    if (!m_file->isFileValid()) {
        LOGE("file [%s] not valid", m_path.c_str());
    } else {
        // 获取有效内容大小
        checkDataValid();
        auto ptr = (uint8_t *) m_file->getMemory();
        if (m_actualSize > 0) {
            // 有内容
            MMBuffer inputBuffer(ptr + Fixed32Size, m_actualSize, MMBufferNoCopy);
            clearDictionary(m_dic);
            // 读取文件中数据填充m_dic
            MiniPBCoder::decodeMap(*m_dic, inputBuffer);
            // 创建写入文件辅助类，从映射内存偏移4字节开始（头4字节是存内容大小），可写空间为文件大小-4字节
            m_output = new CodedOutputData(ptr + Fixed32Size, m_file->getFileSize() - Fixed32Size);
            // 定位到文件实际内容尾部，后续直接从尾部添加KV对
            m_output->seek(m_actualSize);
        } else {
            m_output = new CodedOutputData(ptr + Fixed32Size, m_file->getFileSize() - Fixed32Size);
        }
        auto count = m_dic->size();
        LOGI("loaded [%s] with %zu key-values", m_mmapID.c_str(), count);
    }
}

void EZKV::checkDataValid() {
    // 读取文件前4byte获得内容大小
    m_actualSize = readActualSize();
    LOGD("checkDataValid: m_actualSize = %u", m_actualSize);
}

size_t EZKV::readActualSize() {
    uint32_t actualSize = 0;
    memcpy(&actualSize, m_file->getMemory(), Fixed32Size);
    return actualSize;
}

MMBuffer EZKV::getDataForKey(EZKVKey_t key) {
    auto itr = m_dic->find(key);
    if (itr != m_dic->end()) {
        auto basePtr = (uint8_t *) (m_file->getMemory()) + Fixed32Size;
        return itr->second.toMMBuffer(basePtr);
    }
    MMBuffer nan;
    return nan;
}

bool EZKV::setDataForKey(MMBuffer &&data, EZKVKey_t key, bool isDataHolder) {
    if ((!isDataHolder && data.length() == 0) || isKeyEmpty(key)) {
        return false;
    }

    auto itr = m_dic->find(key);
    if (itr != m_dic->end()) {
        // 有缓存
        auto ret = appendDataWithKey(data, itr->second, isDataHolder);
        if (!ret.first) {
            return false;
        }
        itr->second = std::move(ret.second);
    } else {
        // 无缓存
        auto ret = appendDataWithKey(data, key, isDataHolder);
        if (!ret.first) {
            return false;
        }
        m_dic->emplace(key, std::move(ret.second));
    }

    return true;
}

bool EZKV::removeDataForKey(EZKVKey_t key) {
    if (isKeyEmpty(key)) {
        return false;
    }

    auto itr = m_dic->find(key);
    if (itr != m_dic->end()) {
        static MMBuffer nan;
        auto ret = appendDataWithKey(nan, itr->second);
        if (ret.first) {
            m_dic->erase(itr);
        }
        return ret.first;
    }

    return false;
}

EZKV::KVHolderRet_t
EZKV::doAppendDataWithKey(const MMBuffer &data, const MMBuffer &keyData, bool isDataHolder, uint32_t originKeyLength) {
    // isDataHolder：avoid memory copying when encoding string & buffer。
    // 存string和bytes时须为true，额外存储value length，取的时候，先获取一次value length以构造对象。
    auto isKeyEncoded = (originKeyLength < keyData.length());
    auto keyLength = static_cast<uint32_t>(keyData.length());
    auto valueLength = static_cast<uint32_t>(data.length());
    if (isDataHolder) {
        valueLength += pbRawVarint32Size(valueLength);
    }
    // 计算该kv的大小
    // size needed to encode the key
    size_t size = isKeyEncoded? keyLength : (keyLength + pbRawVarint32Size(keyLength));
    size += valueLength + pbRawVarint32Size(valueLength);

    bool hasEnoughtSize = ensureMemorySize(size);
    if (!hasEnoughtSize || !isFileValid()) {
        return make_pair(false, KeyValueHolder());
    }

    try {
        if (isKeyEncoded) {
            m_output->writeRawData(keyData);
        } else {
            // 依次写入key length值和key值
            m_output->writeData(keyData);
        }
        if (isDataHolder) {
            m_output->writeRawVarint32((int32_t) valueLength);
        }
        // 依次写入value length值和value值
        m_output->writeData(data);
    } catch (std::exception &e) {
        LOGE("%s", e.what());
        return make_pair(false, KeyValueHolder());
    }

    auto offset = static_cast<uint32_t>(m_actualSize);

    m_actualSize += size;

    writeActualSize(m_actualSize);

    return make_pair(true, KeyValueHolder(originKeyLength, valueLength, offset));
}

EZKV::KVHolderRet_t
EZKV::appendDataWithKey(const MMBuffer &data, EZKVKey_t key, bool isDataHolder) {
    auto keyData = MMBuffer((void *) key.data(), key.size(), MMBufferNoCopy);
    return doAppendDataWithKey(data, keyData, isDataHolder, static_cast<uint32_t>(keyData.length()));
}

EZKV::KVHolderRet_t
EZKV::appendDataWithKey(const MMBuffer &data, const KeyValueHolder &kvHolder, bool isDataHolder) {
    uint32_t keyLength = kvHolder.keySize;
    // size needed to encode the key
    size_t rawKeySize = keyLength + pbRawVarint32Size(keyLength);

    // ensureMemorySize() might change kvHolder.offset, so have to do it early
    {
        auto valueLength = static_cast<uint32_t>(data.length());
        if (isDataHolder) {
            valueLength += pbRawVarint32Size(valueLength);
        }
        // 新增的KV对占的大小
        auto size = rawKeySize + valueLength + pbRawVarint32Size(valueLength);
        bool hasEnoughtSize = ensureMemorySize(size);
        if (!hasEnoughtSize) {
            return make_pair(false, KeyValueHolder());
        }
    }
    auto basePtr = (uint8_t *) m_file->getMemory() + Fixed32Size;
    MMBuffer keyData(basePtr + kvHolder.offset, rawKeySize, MMBufferNoCopy);

    return doAppendDataWithKey(data, keyData, isDataHolder, keyLength);
}

constexpr uint32_t ItemSizeHolder = 0x00ffffff;
constexpr uint32_t ItemSizeHolderSize = 4;

static pair<MMBuffer, size_t> prepareEncode(const EZKVMAP &dic) {
    // make some room for placeholder
    size_t totalSize = ItemSizeHolderSize;
    // 累加KV集合中所有KV对占用大小
    for (auto &itr : dic) {
        auto &kvHolder = itr.second;
        totalSize += kvHolder.computedKVSize + kvHolder.valueSize;
    }
    return make_pair(MMBuffer(), totalSize);
}

// since we use append mode, when -[setData: forKey:] many times, space may not be enough
// try a full rewrite to make space
bool EZKV::ensureMemorySize(size_t newSize) {
    if (!isFileValid()) {
        LOGE("[%s] file not valid", m_mmapID.c_str());
        return false;
    }

    if (newSize >= m_output->spaceLeft() || m_dic->empty()) {
        // 若新增KV所需大小大等剩余可写空间 或 KV集合为空，则进行重整回写
        // try a full rewrite to make space
        auto fileSize = m_file->getFileSize();
        // 计算当前已有KV对所需空间（包含额外4字节）
        auto prepareData = prepareEncode(*m_dic);
        auto sizeOfDic = prepareData.second;
        // 新增KV对总共需要的空间
        size_t lenNeeded = sizeOfDic + Fixed32Size + newSize;
        size_t dicCount = m_dic->size();
        size_t avgItemSize = lenNeeded / std::max<size_t>(1, dicCount);
        // 预计后续还会新增Max(8个KV对, 当前KV数的一半)
        size_t futureUsage = avgItemSize * std::max<size_t>(8, (dicCount + 1) / 2);
        // 1. no space for a full rewrite, double it
        // 2. or space is not large enough for future usage, double it to avoid frequently full rewrite
        if (lenNeeded >= fileSize || (lenNeeded + futureUsage) >= fileSize) {
            // 文件还需要扩容
            size_t oldSize = fileSize;
            do {
                fileSize *= 2;
            } while (lenNeeded + futureUsage >= fileSize);
            LOGI("extending [%s] file size from %zu to %zu, incoming size:%zu, future usage:%zu",
                    m_mmapID.c_str(), oldSize, fileSize, newSize, futureUsage);

            // 文件扩容和重映射
            // if we can't extend size, rollback to old state
            if (!m_file->truncate(fileSize)) {
                return false;
            }

            // check if we fail to make more space
            if (!isFileValid()) {
                LOGE("[%s] file not valid", m_mmapID.c_str());
                return false;
            }
        }
        // 回写数据
        return doFullWriteBack(move(prepareData));
    }

    return true;
}

// we don't need to really serialize the dictionary, just reuse what's already in the file
static void memmoveDictionary(EZKVMAP &dic, CodedOutputData *output, uint8_t *ptr, size_t totalSize) {
    auto originOutputPtr = output->curWritePointer();
    // make space to hold the fake size of dictionary's serialization result
    auto writePtr = originOutputPtr + ItemSizeHolderSize;
    // reuse what's already in the file
    if (!dic.empty()) {
        // 若大部分kv对在文件中的位置相邻，排布紧凑，以下操作通过将相邻kv对合并成区块再拷贝，可以大幅减少内存复制次数。
        // sort by offset
        vector<KeyValueHolder *> vec;
        vec.reserve(dic.size());
        for (auto &itr : dic) {
            vec.push_back(&itr.second);
        }
        sort(vec.begin(), vec.end(), [](const auto &left, const auto &right) { return left->offset < right->offset; });

        // merge nearby items to make memmove quicker
        vector<pair<uint32_t, uint32_t>> dataSections; // pair(offset, size)
        dataSections.emplace_back(vec.front()->offset, vec.front()->computedKVSize + vec.front()->valueSize);
        for (size_t index = 1, total = vec.size(); index < total; index++) {
            auto kvHolder = vec[index];
            auto &lastSection = dataSections.back();
            // 判断该kv对和上一个kv对是否相邻
            if (kvHolder->offset == lastSection.first + lastSection.second) {
                // 相邻，扩大区块大小
                lastSection.second += kvHolder->offset + kvHolder->valueSize;
            } else {
                // 不相邻，添加新的区块
                dataSections.emplace_back(kvHolder->offset, kvHolder->computedKVSize + kvHolder->valueSize);
            }
        }
        // do the move
        auto basePtr = ptr + Fixed32Size;
        for (auto &section : dataSections) {
            // memmove() should handle this well: src == dst
            memmove(writePtr, basePtr + section.first, section.second);
            writePtr += section.second;
        }

        // update offset
        auto offset = ItemSizeHolderSize;
        for (auto kvHolder : vec) {
            kvHolder->offset = offset;
            offset += kvHolder->computedKVSize + kvHolder->valueSize;
        }
    }
    // hold the fake size of dictionary's serialization result
    // 写入0x00ffffff，按照变长方式编码为 11111111 11111111 11111111 00000111，占4字节
    output->writeRawVarint32(ItemSizeHolder);
    auto writtenSize = static_cast<size_t>(writePtr - originOutputPtr);
    // 定位至内容结尾，后续从尾部添加数据
    output->seek(writtenSize - ItemSizeHolderSize);
}

bool EZKV::doFullWriteBack(std::pair<ezkv::MMBuffer, size_t> prepareData) {
    auto ptr = (uint8_t *) m_file->getMemory();
    auto totalSize = prepareData.second;

    // 重新mmap后，重建CodedOutputData，重置内存地址和大小和偏移量
    delete m_output;
    m_output = new CodedOutputData(ptr + Fixed32Size, m_file->getFileSize() - Fixed32Size);

    memmoveDictionary(*m_dic, m_output, ptr, totalSize);

    m_actualSize = totalSize;

    writeActualSize(m_actualSize);

    // make sure lastConfirmedMetaInfo is saved
    sync(EZKV_SYNC);
    return true;
}

bool EZKV::writeActualSize(size_t size) {
    oldStyleWriteActualSize(size);
    return true;
}

void EZKV::oldStyleWriteActualSize(size_t actualSize) {
    m_actualSize = actualSize;
    memcpy(m_file->getMemory(), &actualSize, Fixed32Size);
    LOGD("oldStyleWriteActualSize: m_actualSize = %u", m_actualSize);
}

