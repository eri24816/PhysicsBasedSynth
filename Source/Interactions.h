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
			// hammer pos relative to string origin
			Vector2<float> posHS = hammer->transform.getWorldPos() - string->transform.getWorldPos();
			// hammer pos relative to string hit point
			Vector2<float> posHP = hammer->transform.getWorldPos() - 
				string->transform.createChild(Vector2<float>{0.0f, string->sampleU(posHS.x)}).getWorldPos();
			
			if (posHP.y > 0){
				auto impulse = posHP.y * youngsModulus * dt;
				string->applyImpulse(posHS.x, impulse);
				hammer->applyImpulse(Vector2<float>{0.0f, 0.0f}, Vector2<float>{0.0f, -impulse});
			}
		}
	};
}