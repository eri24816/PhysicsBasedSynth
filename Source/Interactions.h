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

	// https://ccrma.stanford.edu/~jos/pasp/Nonlinear_Spring_Model.html
	class HammerStringInteraction : public Interaction {
	public:

		const std::shared_ptr<Rigidbody> hammer;

		const std::shared_ptr<String> string;
		const float Q0, p, width;

		HammerStringInteraction(std::shared_ptr<Rigidbody> hammer, Vector2<float> hammerAttachPoint, std::shared_ptr<String> string, float Q0, float p,
			float width=0 
			)
			: hammer(hammer), string(string), Q0(Q0), p(p), width(width)
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
				auto impulse = pow(posHP.y, p) * Q0 * dt;
				string->applyImpulse(posHS.x, impulse, width);
				hammer->applyImpulse(Vector2<float>{0.0f, 0.0f}, Vector2<float>{0.0f, -impulse});
			}
		}
	};
}