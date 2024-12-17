#pragma once

#include <algorithm>
#include <cstddef>
#include <format>
#include <iterator>
#include <memory>
#include <stdexcept>

namespace stl {

template <typename T, std::size_t N>
class arr {
 public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  constexpr arr() = default;

  template <typename... Args>
  constexpr arr(Args&&... args)
    requires(sizeof...(args) == N)
      : _buffer{std::forward<Args>(args)...} {}

  template <typename InputIt>
  constexpr arr(InputIt first, InputIt last)
    requires std::input_iterator<InputIt> &&
             std::convertible_to<
                 typename std::iterator_traits<InputIt>::value_type,
                 T>
  {
    if (std::distance(first, last) != N) {
      throw std::out_of_range(
          "arr::arr: Iterator range size does not match arr size");
    }
    std::uninitialized_copy(first, last, _buffer);
  }

  auto at(size_type pos) -> reference {
    if (pos >= N) {
      throw std::out_of_range(
          std::format("arr::at: position {} out of range {}", pos, N));
    }
    return _buffer[pos];
  }

  auto at(size_type pos) const -> const_reference {
    if (pos >= N) {
      throw std::out_of_range(
          std::format("arr::at: position {} out of range {}", pos, N));
    }
    return _buffer[pos];
  }

  constexpr auto operator[](size_type pos) -> reference {
    return _buffer[pos];
  }

  constexpr auto operator[](size_type pos) const -> const_reference {
    return _buffer[pos];
  }

  constexpr auto front() -> reference {
    return _buffer[0];
  }

  constexpr auto front() const -> const_reference {
    return _buffer[0];
  }

  constexpr auto back() -> reference {
    return _buffer[N - 1];
  }

  constexpr auto back() const -> const_reference {
    return _buffer[N - 1];
  }

  constexpr auto data() noexcept -> pointer {
    return _buffer;
  }

  constexpr auto data() const noexcept -> const_pointer {
    return _buffer;
  }

  constexpr auto begin() noexcept -> iterator {
    return _buffer;
  }

  constexpr auto begin() const noexcept -> const_iterator {
    return _buffer;
  }

  constexpr auto cbegin() const noexcept -> const_iterator {
    return _buffer;
  }

  constexpr auto end() noexcept -> iterator {
    return _buffer + N;
  }

  constexpr auto end() const noexcept -> const_iterator {
    return _buffer + N;
  }

  constexpr auto cend() const noexcept -> const_iterator {
    return _buffer + N;
  }

  constexpr auto rbegin() noexcept -> reverse_iterator {
    return reverse_iterator(end());
  }

  constexpr auto rbegin() const noexcept -> const_reverse_iterator {
    return const_reverse_iterator(end());
  }

  constexpr auto crbegin() const noexcept -> const_reverse_iterator {
    return const_reverse_iterator(end());
  }

  constexpr auto rend() noexcept -> reverse_iterator {
    return reverse_iterator(begin());
  }

  constexpr auto rend() const noexcept -> const_reverse_iterator {
    return const_reverse_iterator(begin());
  }

  constexpr auto crend() const noexcept -> const_reverse_iterator {
    return const_reverse_iterator(begin());
  }

  constexpr auto empty() const noexcept -> bool {
    return N == 0;
  }

  constexpr auto size() const noexcept -> size_type {
    return N;
  }

  constexpr auto max_size() const noexcept -> size_type {
    return N;
  }

  constexpr auto fill(const T& value) -> void {
    std::uninitialized_fill(_buffer, _buffer + N, value);
  }

  constexpr auto swap(arr& other) noexcept -> void {
    std::swap_ranges(begin(), end(), other.begin());
  }

 private:
  T _buffer[N];
};

}  // namespace stl
