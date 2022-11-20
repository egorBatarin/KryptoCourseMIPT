#include <fcntl.h>
#include <sys/mman.h>
#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <sys/stat.h>
#include "fileio.hpp"
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <type_traits>

namespace fileio {

static_assert(SegSize % 4096 == 0);

File::File(const char* filename, const off_t segOffset) : m_fileFlags(O_RDONLY), m_mmapFlags(PROT_READ) {  

    m_fd = open(filename, m_fileFlags, S_IRUSR);
    if (m_fd < 0) {
        perror("open file error");
        exit(errno);
    }

    struct stat statbuf;
    if (fstat(m_fd, &statbuf) < 0) {
        perror("fstat error");
        exit(errno);
    }

    m_segOffset = segOffset * SegSize;
    m_fileSize = statbuf.st_size;

    if (m_segOffset >= m_fileSize) {
        perror("segOffset out of range");
        exit(-1);
    }

    m_segSize = (m_segOffset + SegSize <= m_fileSize)? SegSize : m_fileSize - m_segOffset;

    m_buf = mmap(NULL, m_segSize, m_mmapFlags, MAP_SHARED, m_fd, m_segOffset);
    if (m_buf == MAP_FAILED) {
        perror("map failure");
        exit(errno);
    }
}

File::File(const char* filename, const size_t size) : m_fileFlags(O_CREAT | O_RDWR | O_TRUNC), m_mmapFlags(PROT_READ | PROT_WRITE) {  

    m_fd = open(filename, m_fileFlags, S_IWUSR | S_IRUSR);
    if (m_fd < 0) {
        perror("open file error");
        exit(errno);
    }

    if (lseek(m_fd, size, SEEK_SET) == -1) {
        perror("seek error");
        exit(errno);
    }
    /*
    printf("printf\n");
    char abb[2] = "a";
    write(m_fd, abb, 2);
    */

    struct stat statbuf;
    if (fstat(m_fd, &statbuf) < 0) {
        perror("fstat error");
        exit(errno);
    }

    m_segOffset = 0;
    m_fileSize = statbuf.st_size;

    m_segSize = SegSize;

    m_buf = mmap(NULL, m_segSize, m_mmapFlags, MAP_SHARED, m_fd, m_segOffset);
    if (m_buf == MAP_FAILED) {
        perror("map failure");
        exit(errno);
    }
}

File::~File() {
    if (munmap(m_buf, m_segSize) == -1) {
        perror("munmap error");
        exit(errno);
    }
    if (close(m_fd) == -1) {
        perror("close file error");
        exit(errno);
    }
}

void* File::nextSegment() {
    // correct
    if (m_segOffset + SegSize >= m_fileSize) {
        return nullptr;
    }
    
    if (munmap(m_buf, m_segSize) == -1) {
        perror("munmap error");
        exit(errno);
    }

    m_segOffset += SegSize;
    m_segSize = (m_segOffset + SegSize <= m_fileSize)? SegSize : m_fileSize - m_segOffset;

    m_buf = mmap(NULL, m_segSize, m_mmapFlags, MAP_SHARED, m_fd, m_segOffset);
    if (m_buf == MAP_FAILED) {
        perror("map failure");
        exit(errno);
    }
    return m_buf;
}

void* File::prevSegment() {
    // correct
    if (m_segOffset - SegSize < 0) {
        return nullptr;
    }
    
    if (munmap(m_buf, m_segSize) == -1) {
        perror("munmap error");
        exit(errno);
    }

    m_segOffset -= SegSize;
    m_segSize = SegSize;

    m_buf = mmap(NULL, m_segSize, m_mmapFlags, MAP_SHARED, m_fd, m_segOffset);
    if (m_buf == MAP_FAILED) {
        perror("map failure");
        exit(errno);
    }
    return m_buf;
}

void* File::getSegment() const {
    assert(m_buf != nullptr);
    return m_buf;
}

size_t File::getFileSize() const {
    return m_fileSize;
}

size_t File::getSegSize() const {
    return m_segSize;
}

ReadOnly::ReadOnly(const char* filename, off_t segOffset) : File(filename, segOffset) {}

ReadOnly::~ReadOnly() {}

unsigned char ReadOnly::operator[] (const size_t idx) {
    assert(idx < SegSize);
    unsigned char* ptr = (unsigned char*)((uint64_t)m_buf + idx);
    return *ptr;
}

size_t ReadOnly::copySegment(size_t size, off_t offset, void* ptr) const {
    if (size > m_segSize - offset) {
        ptr = nullptr;
        return 0;
    }
    
    void* src = (void*)((uint64_t)m_buf + offset);
    ptr = memcpy(ptr, src, size);
    assert(ptr != nullptr);
    return size;
}

void* ReadOnly::getSegment() const {
    return File::getSegment();
}

void* ReadOnly::prevSegment() {
    return File::prevSegment();
}

void* ReadOnly::nextSegment() {
    return File::nextSegment();
}

WriteOnly::WriteOnly(const char* filename, size_t size) : File(filename, size) {}
WriteOnly::~WriteOnly() {}

size_t WriteOnly::write(size_t size, void* ptr) {
    const auto new_size = m_segOffset + size;
    if (new_size > m_fileSize) {
        auto seek_res = lseek(m_fd, new_size, SEEK_SET);
        if (seek_res == -1) {
            perror("seek error");
            exit(errno);
        }
        assert(seek_res == new_size);
        m_fileSize = seek_res;
    }

    size_t offset = 0;
    auto* ptr_tmp = m_buf;
    while(offset != size) {
        assert(ptr_tmp == m_buf);
        auto block = (size - offset < SegSize)? size - offset : SegSize;
        memcpy(m_buf, (unsigned char*)ptr + offset, block);
        offset += block;
        if (block == SegSize) {
            ptr_tmp = File::nextSegment();
        }
    }

}





} // namespace fileio