#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "box.hpp"

namespace stl {

template <typename T>
class vec {
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

  vec() : _size(0), _capacity(0), _buffer(nullptr) {}

  explicit vec(size_type count)
      : _size(count), _capacity(count), _buffer(stl::make_box<T[]>(count)) {
    std::uninitialized_default_construct_n(_buffer.get(), count);
  }

  vec(size_type count, const T& value)
    requires std::copyable<T>
      : _size(count), _capacity(count), _buffer(stl::make_box<T[]>(count)) {
    std::uninitialized_fill_n(_buffer.get(), count, value);
  }

  vec(const vec& other)
      : _size(other._size),
        _capacity(other._size),
        _buffer(stl::make_box<T[]>(other._size)) {
    std::uninitialized_copy_n(other._buffer.get(), other._size, _buffer.get());
  }

  vec(vec&& other) noexcept
      : _size(std::exchange(other._size, 0)),
        _capacity(std::exchange(other._capacity, 0)),
        _buffer(std::move(other._buffer)) {}

  vec(std::initializer_list<T> init)
    requires std::copyable<T>
      : _size(init.size()),
        _capacity(init.size()),
        _buffer(stl::make_box<T[]>(init.size())) {
    std::uninitialized_copy(init.begin(), init.end(), _buffer.get());
  }

  template <typename Iterator>
  vec(Iterator first, Iterator last)
    requires std::input_iterator<Iterator> &&
                 std::constructible_from<
                     T,
                     typename std::iterator_traits<Iterator>::value_type>
      : _size(std::distance(first, last)),
        _capacity(_size),
        _buffer(stl::make_box<T[]>(_size)) {
    std::uninitialized_copy(first, last, _buffer.get());
  }

  ~vec() {
    clear();
  }

  auto operator=(const vec& other) -> vec& {
    if (this != &other) {
      clear();
      if (other._size > _capacity) {
        _buffer = stl::make_box<T[]>(other._size);
        _capacity = other._size;
      }
      std::uninitialized_copy_n(other._buffer.get(), other._size,
                                _buffer.get());
      _size = other._size;
    }
    return *this;
  }

  auto operator=(vec&& other) noexcept -> vec& {
    if (this != &other) {
      clear();
      _buffer.reset();
      _buffer = std::move(other._buffer);
      _size = std::exchange(other._size, 0);
      _capacity = std::exchange(other._capacity, 0);
    }
    return *this;
  }

  auto operator=(std::initializer_list<T> init) -> vec& {
    clear();
    if (init.size() > _capacity) {
      _buffer = stl::make_box<T[]>(init.size());
      _capacity = init.size();
    }
    std::uninitialized_copy(init.begin(), init.end(), _buffer.get());
    _size = init.size();
    return *this;
  }

  auto assign(size_type count, const T& value) -> void {
    clear();
    if (count > _capacity) {
      _buffer = stl::make_box<T[]>(count);
      _capacity = count;
    }
    std::uninitialized_fill_n(_buffer.get(), count, value);
    _size = count;
  }

  template <typename InputIt>
  auto assign(InputIt first, InputIt last) -> void {
    clear();
    size_type count = std::distance(first, last);
    if (count > _capacity) {
      _buffer = stl::make_box<T[]>(count);
      _capacity = count;
    }
    std::uninitialized_copy(first, last, _buffer.get());
    _size = count;
  }

  auto at(size_type pos) -> reference {
    if (pos >= _size) {
      throw std::out_of_range(
          std::format("vec::at: position {} out of range {}", pos, _capacity));
    }
    return _buffer[pos];
  }

  auto at(size_type pos) const -> const_reference {
    if (pos >= _size) {
      throw std::out_of_range(
          std::format("vec::at: position {} out of range {}", pos, _capacity));
    }
    return _buffer[pos];
  }

  auto operator[](size_type pos) -> reference {
    return _buffer[pos];
  }

  auto operator[](size_type pos) const -> const_reference {
    return _buffer[pos];
  }

  auto front() -> reference {
    return _buffer[0];
  }

  auto front() const -> const_reference {
    return _buffer[0];
  }

  auto back() -> reference {
    return _buffer[_size - 1];
  }

  auto back() const -> const_reference {
    return _buffer[_size - 1];
  }

  auto data() noexcept -> pointer {
    return _buffer.get();
  }

  auto data() const noexcept -> const_pointer {
    return _buffer.get();
  }

  auto begin() noexcept -> iterator {
    return _buffer.get();
  }

  auto begin() const noexcept -> const_iterator {
    return _buffer.get();
  }

  auto end() noexcept -> iterator {
    return _buffer.get() + _size;
  }

  auto end() const noexcept -> const_iterator {
    return _buffer.get() + _size;
  }

  auto rbegin() noexcept -> reverse_iterator {
    return reverse_iterator(end());
  }

  auto rbegin() const noexcept -> const_reverse_iterator {
    return const_reverse_iterator(end());
  }

  auto rend() noexcept -> reverse_iterator {
    return reverse_iterator(begin());
  }

  auto rend() const noexcept -> const_reverse_iterator {
    return const_reverse_iterator(begin());
  }

  auto empty() const noexcept -> bool {
    return _size == 0;
  }

  auto size() const noexcept -> size_type {
    return _size;
  }

  auto capacity() const noexcept -> size_type {
    return _capacity;
  }

  auto reserve(size_type new_cap) -> void {
    if (new_cap > _capacity) {
      auto new_buffer = stl::make_box<T[]>(new_cap);
      if constexpr (std::is_nothrow_move_constructible_v<T>) {
        std::uninitialized_move_n(_buffer.get(), _size, new_buffer.get());
      } else {
        std::uninitialized_copy_n(_buffer.get(), _size, new_buffer.get());
      }
      std::destroy_n(_buffer.get(), _size);
      _buffer = std::move(new_buffer);
      _capacity = new_cap;
    }
  }

  auto shrink_to_fit() -> void {
    if (_size < _capacity) {
      auto new_buffer = stl::make_box<T[]>(_size);
      if constexpr (std::is_nothrow_move_constructible_v<T>) {
        std::uninitialized_move_n(_buffer.get(), _size, new_buffer.get());
      } else {
        std::uninitialized_copy_n(_buffer.get(), _size, new_buffer.get());
      }
      std::destroy_n(_buffer.get(), _size);
      _buffer = std::move(new_buffer);
      _capacity = _size;
    }
  }

  auto clear() noexcept -> void {
    std::destroy_n(_buffer.get(), _size);
    _size = 0;
  }

  auto push_back(const T& value) -> void
    requires std::copyable<T>
  {
    if (_size == _capacity) {
      reserve(_capacity == 0 ? 1 : 2 * _capacity);
    }
    new (_buffer.get() + _size) T(value);
    ++_size;
  }

  auto push_back(T&& value) -> void
    requires std::movable<T>
  {
    if (_size == _capacity) {
      reserve(_capacity == 0 ? 1 : 2 * _capacity);
    }
    new (_buffer.get() + _size) T(std::move(value));
    ++_size;
  }

  template <typename... Args>
  auto emplace_back(Args&&... args) -> reference
    requires std::constructible_from<T, Args...>
  {
    if (_size == _capacity) {
      reserve(_capacity == 0 ? 1 : 2 * _capacity);
    }
    new (_buffer.get() + _size) T(std::forward<Args>(args)...);
    ++_size;
    return back();
  }

  auto pop_back() -> void {
    if (_size > 0) {
      --_size;
      _buffer.get()[_size].~T();
    }
  }

  auto resize(size_type count) -> void {
    if (count > _size) {
      reserve(count);
      std::uninitialized_default_construct_n(_buffer.get() + _size,
                                             count - _size);
    } else if (count < _size) {
      std::destroy_n(_buffer.get() + count, _size - count);
    }
    _size = count;
  }

  auto resize(size_type count, const T& value) -> void {
    if (count > _size) {
      reserve(count);
      std::uninitialized_fill_n(_buffer.get() + _size, count - _size, value);
    } else if (count < _size) {
      std::destroy_n(_buffer.get() + count, _size - count);
    }
    _size = count;
  }

  auto swap(vec& other) noexcept -> void {
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_buffer, other._buffer);
  }

  auto operator==(const vec& other) const -> bool {
    if (_size != other._size) {
      return false;
    }
    return std::equal(begin(), end(), other.begin());
  }

  auto operator<=>(const vec& other) const -> std::strong_ordering
    requires std::three_way_comparable<T>
  {
    return std::lexicographical_compare_three_way(begin(), end(), other.begin(),
                                                  other.end());
  }

 private:
  size_type _size;
  size_type _capacity;
  stl::box<T[]> _buffer;
};

}  // namespace stl
