/*
  ==============================================================================

    PhysicsUtil.h
    Created: 26 Aug 2024 12:31:45pm
    Author:  a931e

  ==============================================================================
*/

#pragma once

namespace InstrumentPhysics
{
	template <typename T> struct Vector2 {
		T x, y;
	};

	template <typename T> Vector2<T> operator+(Vector2<T> a, Vector2<T> b) {
		return Vector2<T>{a.x + b.x, a.y + b.y};
	}

	template <typename T> Vector2<T> operator-(Vector2<T> a, Vector2<T> b) {
		return Vector2<T>{a.x - b.x, a.y - b.y};
	}

	template <typename T> Vector2<T> operator*(Vector2<T> a, T b) {
		return Vector2<T>{a.x* b, a.y* b};
	}

	template <typename T> Vector2<T> operator/(Vector2<T> a, T b) {
		return Vector2<T>{a.x / b, a.y / b};
	}

	template <typename T> T dot(Vector2<T> a, Vector2<T> b) {
		return a.x * b.x + a.y * b.y;
	}

	template <typename T> Vector2<T> operator-(Vector2<T> a) {
		return Vector2<T>{-a.x, -a.y};
	}

}