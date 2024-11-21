#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <format>
#include <type_traits>

#include "box.hpp"

namespace stl {

template <typename T>
class vec {
 public:
  explicit vec() : _size(0), _capacity(0), _buffer(nullptr) {}

  explicit vec(std::size_t size)
    requires std::default_initializable<T>
      : _size(size), _capacity(size), _buffer(stl::make_box<T[]>(size)) {}

  explicit vec(std::size_t size, const T& value)
    requires std::copyable<T>
      : _size(size), _capacity(size), _buffer(stl::make_box<T[]>(size)) {
    std::fill(_buffer.get(), _buffer.get() + _size, value);
  }

  explicit vec(const vec& other)
    requires std::copyable<T>
      : _size(other._size),
        _capacity(other._size),
        _buffer(stl::make_box<T[]>(other._size)) {
    std::copy(other._buffer.get(), other._buffer.get() + other._size,
              _buffer.get());
  }

  explicit vec(vec&& other) noexcept
      : _size(std::exchange(other._size, 0)),
        _capacity(std::exchange(other._capacity, 0)),
        _buffer(std::exchange(other._buffer, nullptr)) {}

  vec(std::initializer_list<T> init)
    requires std::copyable<T>
      : _size(init.size()),
        _capacity(init.size()),
        _buffer(stl::make_box<T[]>(init.size())) {
    std::copy(init.begin(), init.end(), _buffer.get());
  }

  auto assign(std::size_t count, const T& value)
    requires std::copyable<T>
  {
    if (count > _capacity) {
      _buffer = stl::make_box<T[]>(count);
      _capacity = count;
    }

    _size = count;
    std::fill(_buffer.get(), _buffer.get() + _size, value);
  }

  auto at(std::size_t index) -> T& {
    if (index >= _size)
      throw std::out_of_range(std::format(
          "vec::at access out of bounds: index {} exceeds capacity {}", index,
          _capacity));
    return _buffer[index];
  }

  auto at(std::size_t index) const -> const T& {
    if (index >= _size)
      throw std::out_of_range(std::format(
          "vec::at access out of bounds: index {} exceeds capacity {}", index,
          _capacity));
    return _buffer[index];
  }

  auto operator[](std::size_t pos) -> T& {
    return _buffer[pos];
  }

  auto operator[](std::size_t pos) const -> const T& {
    return _buffer[pos];
  }

  auto front() -> T& {
    return _buffer[0];
  }

  auto front() const -> const T& {
    return _buffer[0];
  }

  auto back() -> T& {
    return _buffer[_size - 1];
  }

  auto back() const -> const T& {
    return _buffer[_size - 1];
  }

  auto data() noexcept -> T* {
    return _buffer.get();
  }

  auto data() const noexcept -> const T* {
    return _buffer.get();
  }

  auto begin() noexcept -> T* {
    return _buffer.get();
  }

  auto begin() const noexcept -> const T* {
    return _buffer.get();
  }

  auto end() noexcept -> T* {
    return _buffer.get() + _size;
  }

  auto end() const noexcept -> const T* {
    return _buffer.get() + _size;
  }

  auto empty() const noexcept -> bool {
    return _size == 0;
  }

  auto size() const noexcept -> std::size_t {
    return _size;
  }

  auto capacity() const noexcept -> std::size_t {
    return _capacity;
  }

  auto reserve(std::size_t new_cap) -> void {
    if (new_cap > _capacity) {
      auto new_buffer = stl::make_box<T[]>(new_cap);
      std::move(begin(), end(), new_buffer.get());
      _buffer = std::move(new_buffer);
      _capacity = new_cap;
    }
  }

  auto shrink_to_fit() -> void {
    if (_size < _capacity) {
      auto new_buffer = stl::make_box<T[]>(_size);
      std::move(begin(), end(), new_buffer.get());
      _buffer = std::move(new_buffer);
      _capacity = _size;
    }
  }

  auto clear() noexcept -> void {
    _size = 0;
  }

  auto push_back(const T& value) -> void
    requires std::copyable<T>
  {
    if (_size == _capacity)
      reserve(_capacity == 0 ? 1 : 2 * _capacity);
    _buffer[_size++] = value;
  }

  auto push_back(T&& value) -> void
    requires std::movable<T>
  {
    if (_size == _capacity)
      reserve(_capacity == 0 ? 1 : 2 * _capacity);
    _buffer[_size++] = std::move(value);
  }

  template <typename... Args>
    requires std::is_constructible_v<T, Args...>
  auto emplace_back(Args&&... args) -> T& {
    if (_size == _capacity) {
      reserve(_capacity == 0 ? 1 : 2 * _capacity);
    }
    new (data() + _size) T(std::forward<Args>(args)...);
    return (*this)[_size++];
  }

  auto pop_back() -> void {
    if (_size > 0)
      --_size;
  }

  auto resize(std::size_t count, const T& value = T{}) -> void
    requires std::copyable<T>
  {
    if (count > _capacity)
      reserve(count);
    if (count > _size) {
      std::fill(begin() + _size, begin() + count, value);
    }
    _size = count;
  }

  auto swap(vec& other) noexcept -> void {
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_buffer, other._buffer);
  }

  auto operator=(const vec& other) -> vec&
    requires std::copyable<T>
  {
    if (this != &other) {
      if (other._size > _capacity) {
        _buffer = stl::make_box<T[]>(other._size);
        _capacity = other._size;
      }
      std::copy(other._buffer.get(), other._buffer.get() + other._size,
                _buffer.get());
      _size = other._size;
    }
    return *this;
  }

  auto operator=(vec&& other) noexcept -> vec& {
    if (this != &other) {
      _buffer.reset();

      _buffer = std::exchange(other._buffer, nullptr);
      _size = std::exchange(other._size, 0);
      _capacity = std::exchange(other._capacity, 0);
    }
    return *this;
  }

  auto operator=(std::initializer_list<T> init) -> vec&
    requires std::copyable<T>
  {
    if (init.size() > _capacity) {
      _buffer = stl::make_box<T[]>(init.size());
      _capacity = init.size();
    }
    _size = init.size();
    std::copy(init.begin(), init.end(), _buffer.get());
    return *this;
  }

  auto operator==(const vec& other) const noexcept -> bool {
    return _size == other._size && std::equal(begin(), end(), other.begin());
  }

  auto operator<=>(const vec& other) const noexcept -> std::strong_ordering
    requires std::three_way_comparable<T>
  {
    auto cmp = std::lexicographical_compare_three_way(
        begin(), end(), other.begin(), other.end());
    if (cmp != 0)
      return cmp;
    return _size <=> other._size;
  }

 private:
  std::size_t _size;
  std::size_t _capacity;
  stl::box<T[]> _buffer;
};

}  // namespace stl
