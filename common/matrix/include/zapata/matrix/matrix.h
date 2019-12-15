#pragma once

#include <valarray>

namespace zpt {
template<typename T>
class matrix {
    static_assert(std::is_arithmetic<T>::value, "Type `T` in `zpt::matrix<T>` must be arithmetic.");

  public:
    matrix(size_t _initial_rows, size_t _initials_cols);
    virtual ~matrix() = default;

  private:
    std::valarray<std::valarray<T>> __underlying;
};
} // namespace zpt
