//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>

/* A simple 2D vector structure.
 *
 * Implements a lot of overloaded operators to ease calculations.
 */
template <typename T>
struct Vector
{
  T x, y;

  Vector() { }
  template <typename OT>
  Vector(OT other_point_type): x(other_point_type.x), y(other_point_type.y)  { }
  Vector(T px, T py): x(px), y(py) { }

  // conversion from another compatible vector type
  template <typename OT>
  Vector(const Vector<OT> &v)
    : x(v.x), y(v.y)
  {
  }

  template <typename OT>
  Vector<T> operator +=(Vector<OT> other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  template <typename OT>
  Vector<T> operator -=(Vector<OT> other)
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  template <typename OT>
  Vector<T> operator *=(OT number)
  {
    x *= number;
    y *= number;
    return *this;
  }

  template <typename OT>
  Vector<T> operator /=(OT number)
  {
    x /= number;
    y /= number;
    return *this;
  }

  Vector<T> operator -()
  {
    return Vector<T>(-x, -y);
  }
};

template <typename T>
inline Vector<T> operator +(Vector<T> lhs, Vector<T> rhs)
{
  return Vector<T>(lhs.x + rhs.x, lhs.y + rhs.y);
}

template <typename T>
inline Vector<T> operator -(Vector<T> lhs, Vector<T> rhs)
{
  return Vector<T>(lhs.x - rhs.x, lhs.y - rhs.y);
}

template <typename T, typename OT>
inline Vector<T> operator *(Vector<T> v, OT number)
{
  return Vector<T>(v.x * number, v.y * number);
}

template <typename T, typename OT>
inline Vector<T> operator *(OT number, Vector<T> v)
{
  return Vector<T>(v.x * number, v.y * number);
}

template <typename T, typename OT>
inline Vector<T> operator %(Vector<T> v, OT number)
{
  return Vector<T>(v.x % number, v.y % number);
}

template <typename T, typename OT>
inline Vector<T> operator /(Vector<T> v, OT number)
{
  return Vector<T>(v.x / number, v.y / number);
}

template <typename T>
inline bool operator !=(Vector<T> lhs, Vector<T> rhs)
{
  return !(lhs == rhs);
}

template <typename T>
inline bool operator ==(Vector<T> lhs, Vector<T> rhs)
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}  

template <>
inline bool operator ==<double>(Vector<double> lhs, Vector<double> rhs)
{
  return std::abs(lhs.x - rhs.x) < 0.001 && std::abs(lhs.y - rhs.y) < 0.001;
}  

template <>
inline bool operator ==<float>(Vector<float> lhs, Vector<float> rhs)
{
  return std::abs(lhs.x - rhs.x) < 0.001 && std::abs(lhs.y - rhs.y) < 0.001;
}  

// utilities

template <typename T>
inline Vector<T> make_vector(T x, T y)
{
  return Vector<T>(x, y);
}

// distance
template <typename T>
inline T dist(Vector<T> v1, Vector<T> v2)
{
  return std::sqrt((v2.x - v1.x) * (v2.x - v1.x)
		   + (v2.y - v1.y) * (v2.y - v1.y));
}

// specialization to make int case work smoother
template <>
inline int dist(Vector<int> v1, Vector<int> v2)
{
  return static_cast<int>(std::sqrt(float((v2.x - v1.x) * (v2.x - v1.x)
					  + (v2.y - v1.y) * (v2.y - v1.y))));
}

// length
template <typename T>
inline T length(Vector<T> v)
{
  return static_cast<T>(std::sqrt(v.x * v.x + v.y * v.y));
}

// clipping
template <typename T>
inline Vector<T> clip(Vector<T> lower, Vector<T> val, Vector<T> upper)
{
  Vector<T> tmp;
  
  if (val.x > upper.x)
    tmp.x = upper.x;
  else if (val.x < lower.x)
    tmp.x = lower.x;
  else
    tmp.x = val.x;

  if (val.y > upper.y)
    tmp.y = upper.y;
  else if (val.y < lower.y)
    tmp.y = lower.y;
  else
    tmp.y = val.y;

  return tmp;
}

// rounding
template <typename T>
inline Vector<T> round(Vector<T> v)
{
  return Vector<T>(round(v.x), round(v.y));
}


// type cast template, e.g. vector_cast<int>(some_float_vector)
template <typename Dest, typename Src>
inline Vector<Dest> vector_cast(Vector<Src> v)
{
  return Vector<Dest>(static_cast<Dest>(v.x), static_cast<Dest>(v.y));
}


#endif
