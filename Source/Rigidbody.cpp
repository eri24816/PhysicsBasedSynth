/*
  ==============================================================================

    Rigidbody.cpp
    Created: 25 Aug 2024 11:54:00pm
    Author:  a931e

  ==============================================================================
*/

#include "Rigidbody.h"
#include "PhysicsUtil.h"

namespace InstrumentPhysics
{
	Rigidbody::Rigidbody(float mass, Transform transform)
		: mass(mass), transform(transform)
	{
		vel = Vector2<float>{ 0.0f, 0.0f };
	}

	void Rigidbody::update(float t, float dt)
	{
		transform.localPos = transform.localPos + vel * dt;
	}

	void Rigidbody::applyImpulse(Vector2<float> pos, float t, Vector2<float> impulse)
	{
		// pos will be used when rotation is implemented
		vel = vel + impulse / mass;
	}
}