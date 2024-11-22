#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace stl {

template <typename T, typename Deleter = std::default_delete<T>>
class box {
 public:
  using pointer = T*;
  using element_type = T;
  using deleter_type = Deleter;

  explicit box(pointer ptr = nullptr) noexcept : _object(ptr) {}

  template <typename... Args>
  explicit box(std::in_place_t, Args&&... args) {
    _object = new T(std::forward<Args>(args)...);
  }

  box(const box&) = delete;

  box(box&& other) noexcept : _object(std::exchange(other._object, nullptr)) {}

  ~box() {
    reset();
  }

  auto reset(pointer ptr = nullptr) -> void {
    if (_object) {
      Deleter{}(_object);
    }
    _object = ptr;
  }

  auto release() noexcept -> pointer {
    return std::exchange(_object, nullptr);
  }

  auto get() const noexcept -> pointer {
    return _object;
  }

  auto operator=(const box&) -> box& = delete;

  auto operator=(box&& other) noexcept -> box& {
    if (this != &other) {
      reset();
      _object = std::exchange(other._object, nullptr);
    }
    return *this;
  }

  explicit operator bool() const noexcept {
    return _object != nullptr;
  }

  auto operator*() const -> T& {
    return *_object;
  }

  auto operator->() const noexcept -> pointer {
    return _object;
  }

 private:
  pointer _object;
};

template <typename T, typename Deleter>
class box<T[], Deleter> {
 public:
  using pointer = T*;
  using element_type = T;
  using deleter_type = Deleter;

  explicit box(pointer ptr = nullptr) noexcept : _object(ptr) {}

  explicit box(std::size_t size) {
    _object = new T[size]();
  }

  box(const box&) = delete;

  box(box&& other) noexcept : _object(std::exchange(other._object, nullptr)) {}

  ~box() {
    reset();
  }

  auto reset(pointer ptr = nullptr) -> void {
    if (_object) {
      Deleter{}(_object);
    }
    _object = ptr;
  }

  auto release() noexcept -> pointer {
    return std::exchange(_object, nullptr);
  }

  auto get() const noexcept -> pointer {
    return _object;
  }

  auto operator=(const box&) -> box& = delete;

  auto operator=(box&& other) noexcept -> box& {
    if (this != &other) {
      reset();
      _object = std::exchange(other._object, nullptr);
    }
    return *this;
  }

  explicit operator bool() const noexcept {
    return _object != nullptr;
  }

  auto operator[](std::size_t index) -> T& {
    return _object[index];
  }

  auto operator[](std::size_t index) const -> const T& {
    return _object[index];
  }

 private:
  pointer _object;
};

template <typename T>
auto make_box(std::size_t size) -> box<T>
  requires std::is_unbounded_array_v<T>
{
  return box<T>(size);
}

template <typename T, typename... Args>
auto make_box(Args&&... args) -> box<T>
  requires(!std::is_unbounded_array_v<T>)
{
  return box<T>(std::in_place, std::forward<Args>(args)...);
}

}  // namespace stl
