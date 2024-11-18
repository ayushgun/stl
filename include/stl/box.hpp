#pragma once

#include <compare>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace stl {

template <typename T>
class box {
 public:
  using U = std::conditional_t<std::is_array_v<T>, std::remove_extent_t<T>, T>;

  explicit box(std::nullptr_t) noexcept : _object(nullptr) {}

  template <typename... Args>
  explicit box(Args&&... args) {
    if constexpr (sizeof...(Args) == 0) {
      _object = new T();
    } else {
      _object = create_object(std::forward<Args>(args)...);
    }
  }

  box(const box&) = delete;

  box(box&& other) noexcept : _object(std::exchange(other._object, nullptr)) {}

  ~box() {
    destroy_object();
  }

  auto release() -> U* {
    return std::exchange(_object, nullptr);
  }

  auto reset(U* ptr = nullptr) -> void {
    destroy_object();
    _object = ptr;
  }

  auto get() const -> U* {
    return _object;
  }

  auto operator*() const
    requires(!std::is_array_v<T>)
  {
    return *_object;
  }

  auto operator->() const
    requires(!std::is_array_v<T>)
  {
    return _object;
  }

  explicit operator bool() const noexcept {
    return _object != nullptr;
  }

  auto operator[](std::size_t index)
      -> U& requires(std::is_array_v<T>) { return _object[index]; }

  auto operator[](std::size_t index) const
      -> const U& requires(std::is_array_v<T>) { return _object[index]; }

  auto operator=(const box&) -> box& = delete;

  auto operator=(box&& other) noexcept -> box& {
    if (this != &other) {
      reset();
      _object = std::exchange(other._object, nullptr);
    }
    return *this;
  }

  auto operator==(const box& other) const -> bool {
    return _object == other._object;
  }

  auto operator<=>(const box& other) const -> std::strong_ordering {
    return _object <=> other._object;
  }

 private:
  U* _object;

  template <typename... Args>
  static auto create_object(Args&&... args) -> U* {
    if constexpr (std::is_array_v<T>) {
      return new U[(std::forward<Args>(args), ...)]();
    } else if constexpr (sizeof...(Args) == 0) {
      return new T();
    } else {
      return new T(std::forward<Args>(args)...);
    }
  }

  auto destroy_object() -> void {
    if (_object) {
      if constexpr (std::is_array_v<T>) {
        delete[] _object;
      } else {
        delete _object;
      }
      _object = nullptr;
    }
  }
};

template <typename T>
auto make_box(T&& value) -> box<std::decay_t<T>> {
  return box<std::decay_t<T>>(std::forward<T>(value));
}

template <typename T, typename... Args>
auto make_box(Args&&... args) -> box<T> {
  static_assert(
      !std::is_array_v<T>,
      "Use make_box for non-array types or arrays with unknown bounds.");
  return box<T>(std::forward<Args>(args)...);
}

template <typename T>
auto make_box(std::size_t size) -> box<T> {
  static_assert(std::is_array_v<T> && std::extent_v<T> == 0,
                "Use make_box only for arrays with unknown bounds.");
  return box<T>(size);
}

}  // namespace stl
