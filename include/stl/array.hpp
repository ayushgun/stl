#pragma once

#include <algorithm>
#include <cstddef>
#include <format>
#include <iterator>
#include <span>
#include <stdexcept>

namespace stl {

template <typename T, std::size_t N>
class array {
 public:
  explicit constexpr array() = default;

  template <typename... U>
    requires(sizeof...(U) == N && (std::is_convertible_v<U, T> && ...))
  constexpr array(U&&... values) : _data{std::forward<U>(values)...} {}

  auto at(std::size_t index) -> T& {
    if (index < N) {
      return _data[index];
    }

    throw std::out_of_range(std::format(
        "array::at out of bounds: index {} exceeds capacity {}", index, N));
  }

  auto at(std::size_t index) const -> const T& {
    if (index < N) {
      return _data[index];
    }

    throw std::out_of_range(std::format(
        "array::at access out of bounds: index {} exceeds capacity {}", index,
        N));
  }

  constexpr auto front() -> T& {
    return _data[0];
  }

  constexpr auto front() const -> const T& {
    return _data[0];
  }

  constexpr auto back() -> T& {
    return _data[N - 1];
  }

  constexpr auto back() const -> const T& {
    return _data[N - 1];
  }

  constexpr auto data() -> T* {
    return _data;
  }

  constexpr auto data() const -> const T* {
    return _data;
  }

  constexpr auto size() const -> std::size_t {
    return N;
  }

  constexpr void fill(const T& value) {
    std::fill(std::begin(_data), std::end(_data), value);
  }

  constexpr auto begin() -> T* {
    return _data;
  }

  constexpr auto begin() const -> const T* {
    return _data;
  }

  constexpr auto cbegin() const -> const T* {
    return _data;
  }

  constexpr auto end() -> T* {
    return _data + N;
  }

  constexpr auto end() const -> const T* {
    return _data + N;
  }

  constexpr auto cend() const -> const T* {
    return _data + N;
  }

  constexpr auto rbegin() -> std::reverse_iterator<T*> {
    return std::reverse_iterator<T*>(end());
  }

  constexpr auto rbegin() const -> std::reverse_iterator<const T*> {
    return std::reverse_iterator<const T*>(end());
  }

  constexpr auto crbegin() const -> std::reverse_iterator<const T*> {
    return std::reverse_iterator<const T*>(cend());
  }

  constexpr auto rend() -> std::reverse_iterator<T*> {
    return std::reverse_iterator<T*>(begin());
  }

  constexpr auto rend() const -> std::reverse_iterator<const T*> {
    return std::reverse_iterator<const T*>(begin());
  }

  constexpr auto crend() const -> std::reverse_iterator<const T*> {
    return std::reverse_iterator<const T*>(cbegin());
  }

  constexpr auto operator[](std::size_t idx) -> T& {
    return _data[idx];
  }

  constexpr auto operator[](std::size_t idx) const -> const T& {
    return _data[idx];
  }

  constexpr auto operator<=>(const array&) const = default;

 private:
  T _data[N];
};

template <typename T, typename... U>
array(T, U...) -> array<T, 1 + sizeof...(U)>;

}  // namespace stl
