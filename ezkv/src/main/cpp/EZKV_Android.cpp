//
// Created by chidh on 2021/3/25.
//

#include "EZKV.h"
#include "MemoryFile.h"
#include "PBUtility.h"

using namespace std;
using namespace ezkv;

extern std::string mmapedKVKey(const std::string &mmapID);
extern EZKVPath_t mappedKVPathWithID(const std::string &mmapID);

EZKV::EZKV(const string &mmapID)
        : m_mmapID(mmapedKVKey(mmapID))
        , m_path(mappedKVPathWithID(mmapID))
        , m_dic(nullptr)
        , m_file(new MemoryFile(m_path)) {
    m_actualSize = 0;
    m_output = nullptr;

    m_dic = new EZKVMAP();

    loadFromFile();
}
