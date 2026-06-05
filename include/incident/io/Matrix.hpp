#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <vector>

namespace exx::incident {

template<typename T>
class Matrix {
public:
    using value_type = T;

    explicit Matrix(std::size_t rows, std::size_t cols)
        : _rows(rows), _cols(cols), _storage(rows * cols) {}

    std::size_t rows() const noexcept { return _rows; }
    std::size_t cols() const noexcept { return _cols; }

    const T* data() const { return _storage.data(); }

    T& operator()(std::size_t i, std::size_t j)
        requires (!std::is_same_v<T, bool>)
    { return _storage[i * _cols + j]; }
    const T& operator()(std::size_t i, std::size_t j) const
        requires (!std::is_same_v<T, bool>)
    { return _storage[i * _cols + j]; }

    bool operator()(std::size_t i, std::size_t j) const
        requires (std::is_same_v<T, bool>)
    { return _storage[i * _cols + j]; }

    void set(std::size_t i, std::size_t j, bool v)
        requires (std::is_same_v<T, bool>)
    { _storage[i * _cols + j] = v; }

private:
    std::size_t _rows, _cols;
    std::vector<T> _storage;
};

} // namespace exx::incident

#endif // MATRIX_HPP
