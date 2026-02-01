#pragma once

#include <cassert>
#include <array>

enum class Array2DStoreOrder
{
    XY,
    YX,
};

template<typename T, std::size_t X, std::size_t Y, Array2DStoreOrder O = Array2DStoreOrder::XY>
class Array2D
{
public:
    static_assert(O == Array2DStoreOrder::XY || O == Array2DStoreOrder::YX, "Unsupported Array2DStoreOrder");

    using value_type = T;
    using size_type  = std::size_t;

    static constexpr size_type XSize = X;
    static constexpr size_type YSize = Y;
    static constexpr size_type Volume = X * Y;
    static constexpr Array2DStoreOrder Order = O;

    void Fill(const T& v) { m_Elements.fill(v); }

    constexpr       T& At(size_type x, size_type y)       noexcept { return m_Elements[IndexOf(x, y, z)]; }
    constexpr const T& At(size_type x, size_type y) const noexcept { return m_Elements[IndexOf(x, y, z)]; }
    constexpr       T& operator[](size_type index)        noexcept { return m_Elements[index]; }
    constexpr const T& operator[](size_type index)  const noexcept { return m_Elements[index]; }

    constexpr       T* Data()       noexcept { return m_Elements.data(); }
    constexpr const T* Data() const noexcept { return m_Elements.data(); }

    constexpr auto begin()        noexcept { return m_Elements.begin(); }
    constexpr auto begin()  const noexcept { return m_Elements.begin(); }
    constexpr auto end()          noexcept { return m_Elements.end(); }
    constexpr auto end()    const noexcept { return m_Elements.end(); }
    constexpr auto cbegin() const noexcept { return m_Elements.cbegin(); }
    constexpr auto cend()   const noexcept { return m_Elements.cend(); }

private:
    std::array<T, Volume> m_Elements;

    static constexpr size_type IndexOf(size_type x, size_type y) noexcept
    {
#ifndef NDEBUG
        assert(x < X && y < Y);
#endif
        if constexpr (O == Array2DStoreOrder::XY) { return x + (y * X); }
        else                                      { return y + (x * Y); }
    }
};
