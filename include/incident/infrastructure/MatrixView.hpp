#ifndef EXX_MATRIXVIEW_HPP
#define EXX_MATRIXVIEW_HPP

#include <cstddef>

#include "matrix_concepts.hpp"

namespace exx::incident {

template<typename T>
class MatrixView {
public:
    using value_type = T;

    MatrixView(const T* data, std::size_t rows, std::size_t cols)
        : _data(data), _rows(rows), _cols(cols) {}

    template<MatrixProvadingDataView M>
    MatrixView(M&& m) : MatrixView(m.data(), m.rows(), m.cols()) {}

    std::size_t rows() const noexcept { return _rows; }
    std::size_t cols() const noexcept { return _cols; }

    const T& operator()(std::size_t i, std::size_t j) const { return _data[i * _cols + j]; }

private:
    const T* _data;
    std::size_t _rows, _cols;
};

} // namespace exx::incident

#endif // EXX_MATRIXVIEW_HPP
