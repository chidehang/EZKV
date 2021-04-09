//
// Created by chidh on 2021/3/29.
//

#include "MiniPBCoder.h"
#include "EZKVLog.h"

using namespace std;

namespace ezkv {

    MiniPBCoder::MiniPBCoder(const MMBuffer *inputBuffer) {
        m_inputBuffer = inputBuffer;

        m_inputData = new CodedInputData(m_inputBuffer->getPtr(), m_inputBuffer->length());
    }

    void MiniPBCoder::decodeMap(EZKVMAP &dic, const MMBuffer &oData, size_t position) {
        MiniPBCoder oCoder(&oData);
        oCoder.decodeOneMap(dic, position);
    }

    void MiniPBCoder::decodeOneMap(EZKVMAP &dic, size_t position) {
        auto block = [position, this](EZKVMAP &dictionary) {
            if (position) {
                m_inputData->seek(position);
            } else {
                // 跳过ItemSizeHolder
                m_inputData->readInt32();
            }
            // 开始不断读取key-value
            while (!m_inputData->isAtEnd()) {
                KeyValueHolder kvHolder;
                // 读key值
                const auto &key = m_inputData->readString(kvHolder);
                if (key.length() > 0) {
                    // KeyValueHolder中并未直接保存value值，而是保存地址和size，
                    // 因为不同类型的value有不同的解析方式，需要在具体取值时根据调用的方法分别处理。
                    m_inputData->readData(kvHolder);
                    if (kvHolder.valueSize > 0) {
                        // value有值，保存到map中
                        dictionary[key] = move(kvHolder);
                    } else {
                        // 移除key
                        auto itr = dictionary.find(key);
                        if (itr != dictionary.end()) {
                            dictionary.erase(itr);
                        }
                    }
                }
            }
        };

        try {
            EZKVMAP tmpDic;
            block(tmpDic);
            dic.swap(tmpDic);
        } catch (std::exception &exception) {
            LOGE("%s", exception.what());
        }
    }

} // namespace ezkv

