//
// Created by chidh on 2021/3/25.
//

#include "MemoryFile.h"
#include "EZKVLog.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cerrno>
#include <unistd.h>

using namespace std;

namespace ezkv {

    MemoryFile::MemoryFile(const EZKVPath_t &path)
            : m_name(path), m_fd(-1), m_ptr(nullptr), m_size(0) {
        reloadFromFile();
    }

    bool mkPath(const EZKVPath_t &str) {
        char *path = strdup(str.c_str());

        struct stat st = {};
        bool done = false;
        char *slash = path;

        // 逐级检查或创建文件夹
        while (!done) {
            // 截取一层路径
            slash += strspn(slash, "/");
            slash += strcspn(slash, "/");
            // 判断路径是否结束
            done = (*slash == '\0');
            *slash = '\0';

            // stat获取文件状态
            if (stat(path, &st) != 0) {
                // 获取状态失败，ENOENT表示文件不存在。如果不是由于文件不存在导致的失败，则mkPath失败。否则进行
                // 创建目录，若创建失败，则也mkPath失败。
                if (errno != ENOENT || mkdir(path, 0777) != 0) {
                    LOGE("fail to mkPath %s : %s", path, strerror(errno));
                    free(path);
                    return false;
                }
            } else if (!S_ISDIR(st.st_mode)) {
                // 获取状态成功，判断path非目录，则也失败
                LOGE("fail to mkPath %s : %s", path, strerror(ENOTDIR));
                free(path);
                return false;
            }

            *slash = '/';
        }
        free(path);

        return true;
    }

    bool getFileSize(int fd, size_t &size) {
        struct stat st = {};
        if (fstat(fd, &st) != -1) {
            size = (size_t) st.st_size;
            return true;
        }
        return false;
    }

    size_t getPageSize() {
        return static_cast<size_t>(getpagesize());
    }

    // // 内存映射文件三部曲：open->ftruncate->mmap
    void MemoryFile::reloadFromFile() {
        if (isFileValid()) {
            clearMemoryCache();
        }

        // 打开文件
        m_fd = open(m_name.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, S_IRWXU);
        if (m_fd < 0) {
            LOGE("fail to open:%s, %s", m_name.c_str(), strerror(errno));
        } else {
            // 获取文件大小
            ezkv::getFileSize(m_fd, m_size);
            if (m_size < DEFAULT_EZKV_SIZE || (m_size % DEFAULT_EZKV_SIZE != 0)) {
                // 文件大小不是内存页大小整数倍
                size_t roundSize = ((m_size / DEFAULT_EZKV_SIZE) + 1) * DEFAULT_EZKV_SIZE;
                // 调整文件大小
                truncate(roundSize);
            } else {
                // 进行内存映射
                auto ret = mmap();
                if (!ret) {
                    doCleanMemoryCache(true);
                }
            }
        }
    }

    void MemoryFile::doCleanMemoryCache(bool forceClean) {
        if (m_ptr && m_ptr != MAP_FAILED) {
            // 释放分配的内存
            if (munmap(m_ptr, m_size) != 0) {
                LOGE("fail to munmap [%s], %s", m_name.c_str(), strerror(errno));
            }
        }
        m_ptr = nullptr;

        if (m_fd >= 0) {
            // 关闭文件
            if (close(m_fd) != 0) {
                LOGE("fail to close [%s], %s", m_name.c_str(), strerror(errno));
            }
        }
        m_fd = -1;
        m_size = 0;
    }

    bool MemoryFile::truncate(size_t size) {
        if (m_fd < 0) {
            return false;
        }
        if (size == m_size) {
            return true;
        }

        auto oldSize = m_size;
        m_size = size;
        // 对其文件大小为内存页大小整数倍
        if (m_size < DEFAULT_EZKV_SIZE || (m_size % DEFAULT_EZKV_SIZE != 0)) {
            m_size = ((m_size / DEFAULT_EZKV_SIZE) + 1) * DEFAULT_EZKV_SIZE;
        }

        if (ftruncate(m_fd, static_cast<off_t>(m_size)) != 0) {
            // 调整文件大小失败
            LOGE("fail to truncate [%s] to size %zu, %s", m_name.c_str(), m_size, strerror(errno));
            m_size = oldSize;
            return false;
        }

        if (m_size > oldSize) {
            // 使用ftruncate扩容后是稀疏文件，当设备上无可用存储时，使用mmap过程中可能会抛出SIGBUS信号，通过填充'0'来避免。
            if (!zeroFillFile(m_fd, oldSize, m_size - oldSize)) {
                LOGE("fail to zeroFile [%s] to size %zu, %s", m_name.c_str(), m_size, strerror(errno));
                m_size = oldSize;
                return false;
            }
        }

        if (m_ptr) {
            // 如果已经分配过内存，需要释放掉
            if (munmap(m_ptr, oldSize) != 0) {
                LOGE("fail to munmap [%s], %s", m_name.c_str(), strerror(errno));
            }
        }

        // 重新映射内存
        auto ret = mmap();
        if (!ret) {
            doCleanMemoryCache(true);
        }
        return ret;
    }

    // 映射文件和匿名虚拟内存，返回分配内存起始地址
    bool MemoryFile::mmap() {
        m_ptr = (char *) ::mmap(m_ptr, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);
        if (m_ptr == MAP_FAILED) {
            LOGE("fail to mmap [%s], %s", m_name.c_str(), strerror(errno));
            m_ptr = nullptr;
            return false;
        }
        return true;
    }

    bool MemoryFile::msync(SyncFlag syncFlag) {
        if (m_ptr) {
            auto ret = ::msync(m_ptr, m_size, syncFlag? MS_SYNC : MS_ASYNC);
            if (ret == 0) {
                return true;
            }
            LOGE("fail to msync [%s], %s", m_name.c_str(), strerror(errno));
        }
        return false;
    }

    bool zeroFillFile(EZKVFileHandle_t fd, size_t startPos, size_t size) {
        if (fd < 0) {
            return false;
        }

        if (lseek(fd, static_cast<off_t>(startPos), SEEK_SET) < 0) {
            LOGE("fail to lseek fd[%d], error:%s", fd, strerror(errno));
            return false;
        }

        static const char zeros[4096] = {};
        while (size >= sizeof(zeros)) {
            if (write(fd, zeros, sizeof(zeros)) < 0) {
                LOGE("fail to write fd[%d], error:%s", fd, strerror(errno));
                return false;
            }
            size -= sizeof(zeros);
        }
        if (size > 0) {
            if (write(fd, zeros, size) < 0) {
                LOGE("fail to write fd[%d], error:%s", fd, strerror(errno));
                return false;
            }
        }
        return true;
    }
} // namespace ezkv
