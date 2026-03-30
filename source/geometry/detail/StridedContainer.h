#pragma once

// -- Variant A : vector of array

#include <array>
#include <vector>

using VertexData = std::vector<std::array<float, 3>>;
using IndexedFaceData = std::vector<std::array<std::size_t, 3>>

using vec3 = Vector3<float>;
using ivec3 = Vector3<std::size_t>;

namespace
{
    void foo()
    {
        VertexData vertices = {
            { -1, -.5, 0 },
            {  1, -.5, 0 },
            {  0, +.5, 0 },
        };

        for(auto const& vertex : vertices)
        {
            // Provide vec3 c'tor from std::span<float, 3>
            auto direction = vec3(std::span(vertex));
        }
    }
}

// -- Variant B : custom container

#include <cstddef> // size_t
#include <cstring> // memcpy
#include <span>
#include <vector>

template<typename T, std::size_t Stride>
class StridedContainer
{
public:
    T* data() { return data_; }
    T const* data() const { return data_; }

    void reserve(std::size_t numElements) { data_.reserve(numElements*Stride); }

    void resize(std::size_t numElements) { data_.resize(numElements*Stride); }

    void size() const { return data_.size() / Stride; }

    std::span<T, Stride> get(std::size_t index) 
    { 
        return std::span(data_).subspan(index * Stride, Stride); 
    }

    void set(std::size_t index, std::span<T, Stride> elementData)
    {
        constexpr bool useFastCopy = true;

        if constexpr(useFastCopy)
        {
            void* dst = static_cast<void*>(&data_ + index * Stride);
            void const* src = static_cast<void const*>(elementData.data());
            std::memcpy(dst, src, sizeof(T)*Stride);
        }
        else
        {
            std::size_t const elementStart = index * Stride;
            for(auto c = 0; c < Stride; ++c)
            {
                data_[elementStart + c] = elementData[c];
            }
        }
    }

    void push_back(std::span<T, Stride> elementData)
    {
        data_.resize(data_.size() + Stride);
        set(data_.size() - 1, elementData);
    }

private:
    std::vector<T> data_;
}
