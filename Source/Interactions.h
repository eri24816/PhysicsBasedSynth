/*
  ==============================================================================

    Interactions.h
    Created: 26 Aug 2024 11:59:06am
    Author:  a931e

  ==============================================================================
*/

#pragma once

#include <memory>
#include "Rigidbody.h"
#include "String.h"

namespace InstrumentPhysics
{
	class Interaction{
	public:
		Interaction() = default;
		virtual ~Interaction() = default;

		virtual void apply(float t, float dt) = 0;
	};

	class HammerStringInteraction : public Interaction {
	public:

		const std::shared_ptr<Rigidbody> hammer;

		const std::shared_ptr<String> string;
		const float youngsModulus;

		HammerStringInteraction(std::shared_ptr<Rigidbody> hammer, Vector2<float> hammerAttachPoint, std::shared_ptr<String> string, float youngsModulus)
			: hammer(hammer), string(string), youngsModulus(youngsModulus)
		{
		}

		void apply(float t, float dt) override
		{
			Vector2<float> relPos = hammer->transform.getWorldPos() - string->transform.getWorldPos();
			if (relPos.y < 0.0f) {
				auto impulse = -relPos.y * youngsModulus;
				string->applyImpulse(relPos.x, t, impulse);
				hammer->applyImpulse(-relPos, t, Vector2<float>{0.0f, impulse});
			}
		}
	};
}