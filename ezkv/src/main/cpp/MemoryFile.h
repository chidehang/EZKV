//
// Created by chidh on 2021/3/25.
//

#ifndef EZKV_MEMORYFILE_H
#define EZKV_MEMORYFILE_H
#ifdef __cplusplus

#include "EZKVPredef.h"

namespace ezkv {

    class MemoryFile {
        EZKVPath_t m_name;
        EZKVFileHandle_t m_fd;

        void *m_ptr; // 内存映射文件起始地址
        size_t m_size; // 文件大小（也是映射内存大小）

        bool mmap();

        void doCleanMemoryCache(bool forceClean);

    public:
        MemoryFile(const EZKVPath_t &path);

        ~MemoryFile() { doCleanMemoryCache(true); }

        size_t getFileSize() const { return m_size; }

        size_t getActualFileSize();

        void *getMemory() { return m_ptr; }

        EZKVFileHandle_t getFd() { return m_fd; }

        bool truncate(size_t size);

        bool msync(SyncFlag syncFlag);

        void reloadFromFile();

        void clearMemoryCache() { doCleanMemoryCache(false); }

        bool isFileValid() { return m_fd >= 0 && m_size > 0 && m_ptr; }

        explicit MemoryFile(const MemoryFile &other) = delete;
        MemoryFile &operator=(const MemoryFile &other) = delete;
    };

    extern bool mkPath(const EZKVPath_t &path); // 创建目录
    extern size_t getPageSize(); // 获取内存页大小
    extern bool zeroFillFile(EZKVFileHandle_t fd, size_t startPos, size_t size);
} // namespace ezkv

#endif
#endif //EZKV_MEMORYFILE_H
