#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <cmath>
#include <iostream>

template <class T>
struct Vec2 {
  union {
    struct {
      T x, y;
    };
    struct {
      T u, v;
    };
    T raw[2];
  };
  Vec2() : x(0), y(0) {}
  Vec2(T _x, T _y) : x(_x), y(_y) {}
  inline Vec2<T> operator+(const Vec2<T> &r) const {
    return Vec2<T>(x + r.x, y + r.y);
  }
  inline Vec2<T> operator-(const Vec2<T> &r) const {
    return Vec2<T>(x - r.x, y - r.y);
  }
  inline Vec2<T> operator*(T f) const { return Vec2<T>(x * f, y * f); }

  template <class>
  friend std::ostream &operator<<(std::ostream &s, Vec2<T> &v);
};

template <class T>
struct Vec3 {
  union {
    struct {
      T x, y, z;
    };
    struct {
      T ivert, iuv, inorm;
    };
    T raw[3];
  };
  Vec3() : x(0), y(0), z(0) {}
  Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
  inline Vec3<T> operator^(const Vec3<T> &r) const {
    return Vec3<T>(y * r.z - z * r.y, z * r.x - x * r.z, x * r.y - y * r.x);
  }
  inline Vec3<T> operator+(const Vec3<T> &r) const {
    return Vec3<T>(x + r.x, y + r.y, z + r.z);
  }
  inline Vec3<T> operator-(const Vec3<T> &r) const {
    return Vec3<T>(x - r.x, y - r.y, z - r.z);
  }
  inline Vec3<T> operator*(T f) const { return Vec3<T>(x * f, y * f, z * f); }
  inline T operator*(const Vec3<T> &r) const {
    return x * r.x + y * r.y + z * r.z;
  }
  float norm() const { return std::sqrt(x * x + y * y + z * z); }
  Vec3<T> &normalize(T l = 1) {
    *this = (*this) * (l / norm());
    return *this;
  }
  template <class>
  friend std::ostream &operator<<(std::ostream &s, Vec3<T> &v);
};

template <class T>
std::ostream &operator<<(std::ostream &s, Vec2<T> &v) {
  s << "(" << v.x << ", " << v.y << ")\n";
  return s;
}

template <class T>
std::ostream &operator<<(std::ostream &s, Vec3<T> &v) {
  s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
  return s;
}

using Vec2i = Vec2<int>;
using Vec2f = Vec2<float>;

using Vec3i = Vec3<int>;
using Vec3f = Vec3<float>;

#endif