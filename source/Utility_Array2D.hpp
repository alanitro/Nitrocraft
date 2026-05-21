#pragma once

#include <cassert>
#include <array>

namespace nitrocraft::utility
{

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

    using ValueType = T;
    using SizeType  = std::size_t;

    static constexpr SizeType XSize = X;
    static constexpr SizeType YSize = Y;
    static constexpr SizeType Volume = X * Y;
    static constexpr Array2DStoreOrder Order = O;

    void Fill(const T& v) { m_elements.fill(v); }

    constexpr       T& At(SizeType x, SizeType y)       noexcept { return m_elements[IndexOf(x, y)]; }
    constexpr const T& At(SizeType x, SizeType y) const noexcept { return m_elements[IndexOf(x, y)]; }
    constexpr       T& operator[](SizeType index)        noexcept { return m_elements[index]; }
    constexpr const T& operator[](SizeType index)  const noexcept { return m_elements[index]; }

    constexpr       T* Data()       noexcept { return m_elements.data(); }
    constexpr const T* Data() const noexcept { return m_elements.data(); }

    constexpr auto begin()        noexcept { return m_elements.begin(); }
    constexpr auto begin()  const noexcept { return m_elements.begin(); }
    constexpr auto end()          noexcept { return m_elements.end(); }
    constexpr auto end()    const noexcept { return m_elements.end(); }
    constexpr auto cbegin() const noexcept { return m_elements.cbegin(); }
    constexpr auto cend()   const noexcept { return m_elements.cend(); }

private:
    std::array<T, Volume> m_elements;

    static constexpr SizeType IndexOf(SizeType x, SizeType y) noexcept
    {

        assert(x < X && y < Y);

        if constexpr (O == Array2DStoreOrder::XY) { return x + (y * X); }
        else                                      { return y + (x * Y); }
    }
};

} // namespace nitrocraft::utility
