#pragma once

#ifndef ARENAALLOCATOR_H
#define ARENAALLOCATOR_H

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

template <std::size_t DefaultChunkSize = 64 * 1024>
class ArenaAllocator {
public:
    static constexpr size_t CHUNK_SIZE = DefaultChunkSize;

    ArenaAllocator() : current_(nullptr), remaining_(0) {}
    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    ArenaAllocator(ArenaAllocator&& other) noexcept
        : chunks_(std::move(other.chunks_)),
          current_(std::exchange(other.current_, nullptr)),
          remaining_(std::exchange(other.remaining_, 0)) {
    }

    ArenaAllocator& operator=(ArenaAllocator&& other) noexcept {
        if (this != &other) {
            chunks_ = std::move(other.chunks_);
            current_ = std::exchange(other.current_, nullptr);
            remaining_ = std::exchange(other.remaining_, 0);
        }
        return *this;
    }

    ~ArenaAllocator() = default;

    /**
     * Allocate and construct an object of type T
     *
     * @tparam T The type to allocate
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return Pointer to constructed object
     */
    template<typename T, typename... Args>
    [[nodiscard]] T* allocate(Args&&... args) {
        void* ptr = allocate_raw(sizeof(T), alignof(T));
        return std::construct_at(static_cast<T*>(ptr), std::forward<Args>(args)...);
    }

    /**
     * Reset the arena, freeing all allocated memory
     */
    void reset() noexcept {
        chunks_.clear();
        current_ = nullptr;
        remaining_ = 0;
    }

    /**
     * Get total allocated memory (including overhead)
     */
    [[nodiscard]] size_t total_allocated() const {
        size_t total = 0;
        for (const auto& chunk : chunks_) {
            total += chunk.size;
        }
        return total;
    }

    /**
     * Get number of chunks allocated
     */
    [[nodiscard]] size_t chunk_count() const {
        return chunks_.size();
    }

private:
    /**
     * Represents a memory chunk
     */
    struct Chunk {
        std::unique_ptr<std::byte[]> data;
        size_t size{};
    };

    /**
     * Allocate raw memory with proper alignment
     *
     * @param size Number of bytes to allocate
     * @param alignment Required alignment (must be power of 2)
     * @return Pointer to allocated memory
     */
    void* allocate_raw(size_t size, size_t alignment) {
        void* ptr = current_;
        size_t space = remaining_;

        if (!ptr || !std::align(alignment, size, ptr, space)) {
            // Need a new chunk
            size_t chunk_size = std::max(size + alignment, CHUNK_SIZE);
            chunks_.push_back(Chunk{
                std::make_unique<std::byte[]>(chunk_size),
                chunk_size
            });
            ptr = current_ = chunks_.back().data.get();
            space = remaining_ = chunk_size;
            std::align(alignment, size, ptr, space);
        }

        current_ = static_cast<std::byte*>(ptr) + size;
        remaining_ = space - size;
        return ptr;
    }

    std::vector<Chunk> chunks_;
    std::byte* current_;
    size_t remaining_;
};

#endif // ARENAALLOCATOR_H
