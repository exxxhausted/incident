#ifndef EXX_HASHSTDINJECTION_HPP
#define EXX_HASHSTDINJECTION_HPP

#include <functional>

namespace exx::incident {

template<typename T>
concept HashableByExxIncident =
    requires(const T& v) {
        typename T::CustomHasherProvidedByExxIncident;

        { typename T::CustomHasherProvidedByExxIncident{}(v) }
              -> std::convertible_to<std::size_t>;
    };

} // namespace exx::incident

namespace std {

template<typename T>
    requires exx::incident::HashableByExxIncident<T>
struct hash<T> {
    std::size_t operator()(const T& val) const
    { return typename T::CustomHasherProvidedByExxIncident{}(val); }
};

} // namespace std

#endif // EXX_HASHSTDINJECTION_HPP
