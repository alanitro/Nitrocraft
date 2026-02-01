#pragma once

#include <cassert>
#include <array>

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

    using value_type = T;
    using size_type = std::size_t;

    static constexpr size_type  XSize = X;
    static constexpr size_type  YSize = Y;
    static constexpr size_type  ZSize = Z;
    static constexpr size_type  Volume = X * Y * Z;
    static constexpr Array3DStoreOrder Order = O;

    void Fill(const T& v) { m_Elements.fill(v); }

    constexpr       T& At(size_type x, size_type y, size_type z)       noexcept { return m_Elements[IndexOf(x, y, z)]; }
    constexpr const T& At(size_type x, size_type y, size_type z) const noexcept { return m_Elements[IndexOf(x, y, z)]; }
    constexpr       T& operator[](size_type index)       noexcept { return m_Elements[index]; }
    constexpr const T& operator[](size_type index) const noexcept { return m_Elements[index]; }

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

    static constexpr size_type IndexOf(size_type x, size_type y, size_type z) noexcept
    {
#ifndef NDEBUG
        assert(x < X && y < Y && z < Z);
#endif
        if constexpr (O == Array3DStoreOrder::XYZ) { return x + (y * X) + (z * (X * Y)); }
        else if      (O == Array3DStoreOrder::XZY) { return x + (z * X) + (y * (X * Z)); }
        else if      (O == Array3DStoreOrder::YXZ) { return y + (x * Y) + (z * (Y * X)); }
        else if      (O == Array3DStoreOrder::YZX) { return y + (z * Y) + (x * (Y * Z)); }
        else if      (O == Array3DStoreOrder::ZXY) { return z + (x * Z) + (y * (Z * X)); }
        else                                       { return z + (y * Z) + (x * (Z * Y)); }
    }
};
