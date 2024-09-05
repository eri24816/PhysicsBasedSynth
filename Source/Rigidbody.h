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
		void applyImpulse(Vector2<float> pos, Vector2<float> impulse);
	};

};