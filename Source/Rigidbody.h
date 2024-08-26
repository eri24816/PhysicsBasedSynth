/*
  ==============================================================================

    Rigidbody.h
    Created: 25 Aug 2024 11:54:00pm
    Author:  a931e

  ==============================================================================
*/

#pragma once

#include "Object.h"
#include "PhysicsUtil.h"

namespace InstrumentPhysics
{
	class Rigidbody : public Object
	{
	public:

		Rigidbody(float mass, Transform transform);

		void update(float t, float dt) override;

		float mass;
		Transform transform;
		Vector2<float> vel;
		void applyImpulse(Vector2<float> pos, float t, Vector2<float> impulse);
	};

	class Transform {
	public:
		Transform(Transform* parent, Vector2<float> localPos)
			: parent(parent), localPos(localPos)
		{
			
		};

		Transform* parent;
		Vector2<float> localPos;

		Vector2<float> getWorldPos() const {
			return parent ? parent->getWorldPos() + localPos : localPos;
		}

	};
};