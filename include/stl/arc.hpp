#pragma once

#include <atomic>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace stl {

template <typename T>
class arc {
 public:
  using U = std::remove_extent_t<T>;

  explicit arc(std::nullptr_t) noexcept : _block(nullptr) {}

  explicit arc()
    requires(!std::is_array_v<T>)
      : _block(new control_block()) {}

  explicit arc(std::size_t n)
    requires std::is_unbounded_array_v<T>
      : _block(new control_block(n)) {}

  template <typename... Args>
    requires std::is_constructible_v<T, Args...> && (!std::is_array_v<T>)
  explicit arc(Args&&... args)
      : _block(new control_block(std::forward<Args>(args)...)) {}

  arc(const arc& other) noexcept : _block(other._block) {
    if (_block) {
      _block->add_ref();
    }
  }

  arc(arc&& other) noexcept : _block(std::exchange(other._block, nullptr)) {}

  ~arc() {
    reset();
  }

  auto use_count() const noexcept -> std::size_t {
    return _block ? _block->ref_count() : 0;
  }

  auto unique() const noexcept -> bool {
    return use_count() == 1;
  }

  auto reset() noexcept -> void {
    if (_block && _block->release_ref() == 0) {
      delete _block;
    }
    _block = nullptr;
  }

  auto get() const noexcept -> U* {
    if constexpr (std::is_array_v<T>) {
      return _block ? _block->_object : nullptr;
    } else {
      return _block ? &_block->_object : nullptr;
    }
  }

  explicit operator bool() const noexcept {
    return _block != nullptr;
  }

  auto operator*() const
      -> U& requires(!std::is_array_v<T>) { return _block->_object; }

  auto operator->() const
      -> U* requires(!std::is_array_v<T>) { return &_block->_object; }

  auto operator[](std::size_t index) -> U&
    requires std::is_array_v<T>
  {
    return _block->_object[index];
  }

  auto operator[](std::size_t index) const -> const U&
    requires std::is_array_v<T>
  {
    return _block->_object[index];
  }

  auto operator=(const arc& other) noexcept -> arc& {
    if (this != &other) {
      reset();
      _block = other._block;
      if (_block) {
        _block->add_ref();
      }
    }
    return *this;
  }

  auto operator=(arc&& other) noexcept -> arc& {
    if (this != &other) {
      reset();
      _block = std::exchange(other._block, nullptr);
    }
    return *this;
  }

  auto operator==(const arc& other) const noexcept -> bool {
    return get() == other.get();
  }

  auto operator<=>(const arc& other) const noexcept -> std::strong_ordering {
    return get() <=> other.get();
  }

 private:
  struct control_block {
    template <typename... Args>
      requires std::is_constructible_v<T, Args...> && (!std::is_array_v<T>)
    explicit control_block(Args&&... args)
        : _object(std::forward<Args>(args)...) {}

    explicit control_block(std::size_t n)
      requires std::is_unbounded_array_v<T>
        : _object(new U[n]), _array_size(n) {}

    ~control_block() {
      if constexpr (std::is_unbounded_array_v<T>) {
        delete[] _object;
      }
    }

    auto add_ref() noexcept -> void {
      _ref_count.fetch_add(1, std::memory_order_relaxed);
    }

    auto release_ref() noexcept -> std::size_t {
      return _ref_count.fetch_sub(1, std::memory_order_acq_rel) - 1;
    }

    auto ref_count() const noexcept -> std::size_t {
      return _ref_count.load(std::memory_order_relaxed);
    }

    std::atomic<std::size_t> _ref_count{1};
    std::conditional_t<std::is_unbounded_array_v<T>, U*, U> _object{nullptr};
    std::size_t _array_size{0};
  };

  control_block* _block{nullptr};
};

template <typename T, typename... Args>
auto make_arc(Args&&... args) -> arc<T> {
  return arc<T>(std::forward<Args>(args)...);
}

template <typename T>
auto make_arc(std::size_t n) -> arc<T>
  requires std::is_unbounded_array_v<T>
{
  return arc<T>(n);
}

}  // namespace stl
