#pragma once

#include <atomic>
#include <compare>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace stl {

template <typename T>
class weak_arc;

template <typename T>
class arc {
 public:
  using element_type = std::remove_extent_t<T>;
  using weak_type = weak_arc<T>;

  arc() noexcept : _block(nullptr) {}

  template <typename... Args>
    requires std::is_constructible_v<T, Args...>
  explicit arc(Args&&... args)
      : _block(new control_block(std::forward<Args>(args)...)) {}

  arc(const arc& other) noexcept : _block(other._block) {
    if (_block) {
      _block->add_ref();
    }
  }

  arc(arc&& other) noexcept : _block(std::exchange(other._block, nullptr)) {}

  ~arc() {
    if (_block) {
      _block->release_ref();
    }
  }

  auto use_count() const noexcept -> std::size_t {
    return _block ? _block->ref_count() : 0;
  }

  auto unique() const noexcept -> bool {
    return use_count() == 1;
  }

  auto get() const noexcept -> element_type* {
    return _block ? &_block->_object : nullptr;
  }

  explicit operator bool() const noexcept {
    return _block != nullptr;
  }

  auto operator*() const -> element_type& {
    return _block->_object;
  }

  auto operator->() const -> element_type* {
    return &_block->_object;
  }

  auto operator=(const arc& other) noexcept -> arc& {
    if (this != &other) {
      if (_block) {
        _block->release_ref();
      }
      _block = other._block;
      if (_block) {
        _block->add_ref();
      }
    }
    return *this;
  }

  auto operator=(arc&& other) noexcept -> arc& {
    if (this != &other) {
      if (_block) {
        _block->release_ref();
      }
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
    explicit control_block(Args&&... args)
        : _object(std::forward<Args>(args)...) {}

    ~control_block() = default;

    auto add_ref() noexcept -> void {
      _ref_count.fetch_add(1, std::memory_order_relaxed);
    }

    auto release_ref() noexcept -> void {
      if (_ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        release_weak_ref();
      }
    }

    auto add_weak_ref() noexcept -> void {
      _weak_count.fetch_add(1, std::memory_order_relaxed);
    }

    auto release_weak_ref() noexcept -> void {
      if (_weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        delete this;
      }
    }

    auto ref_count() const noexcept -> std::size_t {
      return _ref_count.load(std::memory_order_relaxed);
    }

    std::atomic<std::size_t> _ref_count{1};
    std::atomic<std::size_t> _weak_count{1};
    T _object;
  };

  control_block* _block{nullptr};

  // Friend declarations
  friend class weak_arc<T>;

  explicit arc(control_block* block) noexcept : _block(block) {
    if (_block) {
      _block->add_ref();
    }
  }
};

template <typename T>
class arc<T[]> {
 public:
  using element_type = std::remove_extent_t<T>;
  using weak_type = weak_arc<T[]>;

  arc() noexcept : _block(nullptr) {}

  explicit arc(std::size_t n) : _block(new control_block(n)) {}

  arc(const arc& other) noexcept : _block(other._block) {
    if (_block) {
      _block->add_ref();
    }
  }

  arc(arc&& other) noexcept : _block(std::exchange(other._block, nullptr)) {}

  ~arc() {
    if (_block) {
      _block->release_ref();
    }
  }

  auto use_count() const noexcept -> std::size_t {
    return _block ? _block->ref_count() : 0;
  }

  auto unique() const noexcept -> bool {
    return use_count() == 1;
  }

  auto get() const noexcept -> element_type* {
    return _block ? _block->_data : nullptr;
  }

  explicit operator bool() const noexcept {
    return _block != nullptr;
  }

  auto operator[](std::size_t index) const -> const element_type& {
    return _block->_data[index];
  }

  auto operator[](std::size_t index) -> element_type& {
    return _block->_data[index];
  }

  auto operator=(const arc& other) noexcept -> arc& {
    if (this != &other) {
      if (_block) {
        _block->release_ref();
      }
      _block = other._block;
      if (_block) {
        _block->add_ref();
      }
    }
    return *this;
  }

  auto operator=(arc&& other) noexcept -> arc& {
    if (this != &other) {
      if (_block) {
        _block->release_ref();
      }
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
    explicit control_block(std::size_t n)
        : _data(new element_type[n]), _size(n) {}

    ~control_block() {
      if (_data) {
        delete[] _data;
      }
    }

    auto add_ref() noexcept -> void {
      _ref_count.fetch_add(1, std::memory_order_relaxed);
    }

    auto release_ref() noexcept -> void {
      if (_ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        if (_data) {
          delete[] _data;
          _data = nullptr;
        }
        release_weak_ref();
      }
    }

    auto add_weak_ref() noexcept -> void {
      _weak_count.fetch_add(1, std::memory_order_relaxed);
    }

    auto release_weak_ref() noexcept -> void {
      if (_weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        delete this;
      }
    }

    auto ref_count() const noexcept -> std::size_t {
      return _ref_count.load(std::memory_order_relaxed);
    }

    std::atomic<std::size_t> _ref_count{1};
    std::atomic<std::size_t> _weak_count{1};
    element_type* _data{nullptr};
    std::size_t _size{0};
  };

  control_block* _block{nullptr};

  friend class weak_arc<T[]>;

  explicit arc(control_block* block) noexcept : _block(block) {
    if (_block) {
      _block->add_ref();
    }
  }
};

template <typename T>
class weak_arc {
 public:
  using element_type = std::remove_extent_t<T>;

  weak_arc() noexcept : _block(nullptr) {}

  weak_arc(const arc<T>& shared) noexcept : _block(shared._block) {
    if (_block) {
      _block->add_weak_ref();
    }
  }

  weak_arc(const weak_arc& other) noexcept : _block(other._block) {
    if (_block) {
      _block->add_weak_ref();
    }
  }

  weak_arc(weak_arc&& other) noexcept
      : _block(std::exchange(other._block, nullptr)) {}

  ~weak_arc() {
    if (_block) {
      _block->release_weak_ref();
    }
  }

  auto use_count() const noexcept -> std::size_t {
    return _block ? _block->ref_count() : 0;
  }

  auto expired() const noexcept -> bool {
    return use_count() == 0;
  }

  auto lock() const noexcept -> arc<T> {
    if (_block && _block->ref_count() > 0) {
      return arc<T>(_block);
    } else {
      return arc<T>();
    }
  }

  auto operator=(const weak_arc& other) noexcept -> weak_arc& {
    if (this != &other) {
      if (_block) {
        _block->release_weak_ref();
      }
      _block = other._block;
      if (_block) {
        _block->add_weak_ref();
      }
    }
    return *this;
  }

  auto operator=(weak_arc&& other) noexcept -> weak_arc& {
    if (this != &other) {
      if (_block) {
        _block->release_weak_ref();
      }
      _block = std::exchange(other._block, nullptr);
    }
    return *this;
  }

 private:
  typename arc<T>::control_block* _block{nullptr};
};

template <typename T>
class weak_arc<T[]> {
 public:
  using element_type = std::remove_extent_t<T>;

  weak_arc() noexcept : _block(nullptr) {}

  weak_arc(const arc<T[]>& shared) noexcept : _block(shared._block) {
    if (_block) {
      _block->add_weak_ref();
    }
  }

  weak_arc(const weak_arc& other) noexcept : _block(other._block) {
    if (_block) {
      _block->add_weak_ref();
    }
  }

  weak_arc(weak_arc&& other) noexcept
      : _block(std::exchange(other._block, nullptr)) {}

  ~weak_arc() {
    if (_block) {
      _block->release_weak_ref();
    }
  }

  auto use_count() const noexcept -> std::size_t {
    return _block ? _block->ref_count() : 0;
  }

  auto expired() const noexcept -> bool {
    return use_count() == 0;
  }

  auto lock() const noexcept -> arc<T[]> {
    if (_block && _block->ref_count() > 0) {
      return arc<T[]>(_block);
    } else {
      return arc<T[]>();
    }
  }

  auto operator=(const weak_arc& other) noexcept -> weak_arc& {
    if (this != &other) {
      if (_block) {
        _block->release_weak_ref();
      }
      _block = other._block;
      if (_block) {
        _block->add_weak_ref();
      }
    }
    return *this;
  }

  auto operator=(weak_arc&& other) noexcept -> weak_arc& {
    if (this != &other) {
      if (_block) {
        _block->release_weak_ref();
      }
      _block = std::exchange(other._block, nullptr);
    }
    return *this;
  }

 private:
  typename arc<T[]>::control_block* _block{nullptr};
};

template <typename T, typename... Args>
auto make_arc(Args&&... args) -> arc<T>
  requires(!std::is_unbounded_array_v<T>)
{
  return arc<T>(std::forward<Args>(args)...);
}

template <typename T>
auto make_arc(std::size_t n) -> arc<T>
  requires std::is_unbounded_array_v<T>
{
  return arc<T>(n);
}

}  // namespace stl
