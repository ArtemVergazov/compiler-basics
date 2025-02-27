#pragma once

#include <cstddef> // std::byte
#include <cstdlib> // size_t, malloc, free
#include <memory> // std::align
#include <utility> // std::forward

class ArenaAllocator {
public:
    ArenaAllocator() = delete;
    ArenaAllocator(const ArenaAllocator &) = delete;
    ArenaAllocator &operator=(const ArenaAllocator &) = delete;

    explicit ArenaAllocator(const size_t bytes) :
        mSize{ bytes },
        mBuffer{ new std::byte[mSize] },
        mOffset{ mBuffer } {}

    template <typename T>
    [[nodiscard]] T *alloc() {
        size_t remainingBytes{ static_cast<size_t>(mBuffer + mSize - mOffset) };
        void *offset{ static_cast<void *>(mOffset) };
        void *const aligned{ std::align(alignof(T), sizeof(T), offset, remainingBytes) };
        if (!aligned) {
            return nullptr;
        }
        mOffset = static_cast<std::byte *>(aligned) + sizeof(T);
        return static_cast<T *>(aligned);
    }

    template <typename T, typename... Args>
    [[nodiscard]] T *emplace(Args &&...args)
    {
        T *const allocatedMemory{ alloc<T>() };
        return new (allocatedMemory) T{ std::forward<Args>(args)... };
    }

private:
    size_t mSize{};
    std::byte *mBuffer{};
    std::byte *mOffset{ mBuffer };
};
