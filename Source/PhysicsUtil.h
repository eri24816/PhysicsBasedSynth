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


	class Transform {
	public:
		Transform(Transform* parent, Vector2<float> localPos)
			: parent(parent), localPos(localPos)
		{

		};

		Transform(float x, float y)
			: parent(nullptr), localPos({ x, y })
		{

		};

		Transform* parent;
		Vector2<float> localPos;

		Vector2<float> getWorldPos() const {
			return parent ? parent->getWorldPos() + localPos : localPos;
		}

		Transform createChild(Vector2<float> localPos) {
			return Transform(this, localPos);
		}

	};

}