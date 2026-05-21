#pragma once

#include <cassert>
#include <array>

namespace nitrocraft::utility
{

enum class Array3DStoreOrder
{
    XYZ,
    XZY,
    YXZ,
    YZX,
    ZXY,
    ZYX,
};

template<typename T, std::size_t X, std::size_t Y, std::size_t Z, Array3DStoreOrder O = Array3DStoreOrder::XYZ>
class Array3D
{
public:
    static_assert(
        O == Array3DStoreOrder::XYZ ||
        O == Array3DStoreOrder::XZY ||
        O == Array3DStoreOrder::YXZ ||
        O == Array3DStoreOrder::YZX ||
        O == Array3DStoreOrder::ZXY ||
        O == Array3DStoreOrder::ZYX,
        "Unsupported Array3DStoreOrder"
    );

    using ValueType = T;
    using SizeType = std::size_t;

    static constexpr SizeType XSize = X;
    static constexpr SizeType YSize = Y;
    static constexpr SizeType ZSize = Z;
    static constexpr SizeType Volume = X * Y * Z;
    static constexpr Array3DStoreOrder Order = O;

    void Fill(const T& v) { m_elements.fill(v); }

    constexpr       T& At(SizeType x, SizeType y, SizeType z)       noexcept { return m_elements[IndexOf(x, y, z)]; }
    constexpr const T& At(SizeType x, SizeType y, SizeType z) const noexcept { return m_elements[IndexOf(x, y, z)]; }
    constexpr       T& operator[](SizeType index)       noexcept { return m_elements[index]; }
    constexpr const T& operator[](SizeType index) const noexcept { return m_elements[index]; }

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

    static constexpr SizeType IndexOf(SizeType x, SizeType y, SizeType z) noexcept
    {
        assert(x < X && y < Y && z < Z);

        if constexpr (O == Array3DStoreOrder::XYZ) { return x + (y * X) + (z * (X * Y)); }
        else if      (O == Array3DStoreOrder::XZY) { return x + (z * X) + (y * (X * Z)); }
        else if      (O == Array3DStoreOrder::YXZ) { return y + (x * Y) + (z * (Y * X)); }
        else if      (O == Array3DStoreOrder::YZX) { return y + (z * Y) + (x * (Y * Z)); }
        else if      (O == Array3DStoreOrder::ZXY) { return z + (x * Z) + (y * (Z * X)); }
        else                                       { return z + (y * Z) + (x * (Z * Y)); }
    }
};

} // namespace nitrocraft::utility
