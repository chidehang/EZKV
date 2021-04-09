//
// Created by chidh on 2021/3/29.
//

#ifndef EZKV_MINIPBCODER_H
#define EZKV_MINIPBCODER_H
#ifdef __cplusplus

#include "EZKVPredef.h"
#include "MMBuffer.h"
#include "CodedInputData.h"
#include "KeyValueHolder.h"

namespace ezkv {

    class MiniPBCoder {
        const MMBuffer *m_inputBuffer = nullptr;
        CodedInputData *m_inputData = nullptr;

        explicit MiniPBCoder(const MMBuffer *inputBuffer);

        void decodeOneMap(EZKVMAP &dic, size_t position);

    public:
        // return empty result if there's any error
        static void decodeMap(EZKVMAP &dic, const MMBuffer &oData, size_t position = 0);

        // just forbid it for possibly misuse
        explicit MiniPBCoder(const MiniPBCoder &other) = delete;
        MiniPBCoder &operator=(const MiniPBCoder &other) = delete;
    };
} // namespace ezkv

#endif
#endif //EZKV_MINIPBCODER_H
