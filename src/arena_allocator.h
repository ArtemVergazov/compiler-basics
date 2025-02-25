#pragma once

#include <cstddef> // std::byte
#include <cstdlib> // size_t, malloc, free

class ArenaAllocator {
public:
    explicit ArenaAllocator(size_t bytes) : mSize(bytes) {
        mBuffer = static_cast<std::byte *>(malloc(mSize));
        mOffset = mBuffer;
    }

    template <typename T>
    [[nodiscard]] T *alloc() {
        T *offset = reinterpret_cast<T *>(mOffset);
        mOffset += sizeof(T);
        return offset;
    }

    ~ArenaAllocator() {
        free(mBuffer);
    }

    ArenaAllocator(const ArenaAllocator &) = delete;
    ArenaAllocator &operator=(const ArenaAllocator &) = delete;

private:
    size_t mSize;
    std::byte *mBuffer;
    std::byte *mOffset;
};
