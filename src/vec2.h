#pragma once

template <typename T>
struct vec2 {
  constexpr vec2() = default;

  template <typename X, typename Y,
            typename = std::enable_if_t<
                std::is_convertible_v<X, T> && std::is_convertible_v<Y, T>>>
  constexpr vec2(X x, Y y) : x(x), y(y) {}

  constexpr vec2(vec2&& other) = default;
  constexpr vec2(const vec2& other) = default;
  constexpr vec2& operator=(vec2&& other) = default;
  constexpr vec2& operator=(const vec2& other) = default;

  template <typename U>
  constexpr explicit vec2(vec2<U> other) : x(other.x), y(other.y) {}

  T x, y;
};

template <typename T>
vec2(T, T) -> vec2<T>;

template <typename T>
constexpr auto operator+(vec2<T> a, vec2<T> b) {
  return vec2<T>{a.x + b.x, a.y + b.y};
}

template <typename T>
constexpr auto operator-(vec2<T> a, vec2<T> b) {
  return vec2<T>{a.x - b.x, a.y - b.y};
}

template <typename S, typename T>
constexpr auto operator*(S s, vec2<T> v) {
  return vec2<T>{s * v.x, s * v.y};
}

template <typename S, typename T>
constexpr auto operator*(vec2<T> v, S s) {
  return vec2<T>{v.x * s, v.y * s};
}

template <typename T>
constexpr bool operator==(vec2<T> a, vec2<T> b) {
  return a.x == b.x && a.y == b.y;
}

template <typename T>
constexpr bool operator<(vec2<T> a, vec2<T> b) {
  if (a.y < b.y) return true;
  if (a.y > b.y) return false;
  return a.x < b.x;
}

template <typename T>
constexpr bool operator!=(vec2<T> a, vec2<T> b) { return !(a == b); }

template <typename T>
constexpr bool operator>(vec2<T> a, vec2<T> b) { return b < a; }

template <typename T>
constexpr bool operator<=(vec2<T> a, vec2<T> b) { return !(b < a); }

template <typename T>
constexpr bool operator>=(vec2<T> a, vec2<T> b) { return !(a < b); }

template <typename T>
struct std::hash<vec2<T>> {
  constexpr auto operator()(vec2<T> v) const {
    return (v.x * 19) ^ (v.y * 37);
  }
};
