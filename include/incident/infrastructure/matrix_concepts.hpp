#ifndef EXX_MATRIX_CONCEPTS_HPP
#define EXX_MATRIX_CONCEPTS_HPP

#include <concepts>

namespace exx::incident {

template<typename M>
concept MatrixLike = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m(i, j) }  -> std::convertible_to<typename M::value_type>;
};

template<typename M>
concept MatrixProvadingDataView = requires(M m, std::size_t i, std::size_t j) {
    typename M::value_type;
    { m.rows() } -> std::convertible_to<std::size_t>;
    { m.cols() } -> std::convertible_to<std::size_t>;
    { m.data() } -> std::convertible_to<const typename M::value_type*>;
};

} // namespace exx::incident

#endif // EXX_MATRIX_CONCEPTS_HPP
