/*
  ==============================================================================

    String.h
    Created: 25 Aug 2024 10:44:29pm
    Author:  a931e

  ==============================================================================
*/

#pragma once

#include <cmath>
#include "Object.h"
#include "PhysicsUtil.h"

namespace InstrumentPhysics {

	constexpr float PI = 3.14159265358979323846;
	constexpr float TWO_PI = 2 * PI;
	constexpr int STRING_MAX_HARMONICS = 50;


	struct SingleStringProfile {
		float length, tension, density, stiffness, damping, harmonics;
	};

	class String : public Object
	{
	public:
		String(float L, float tension, float density, float stiffness, int harmonics, float damping);
		String(SingleStringProfile profile) : String(profile.length, profile.tension, profile.density, profile.stiffness, profile.harmonics, profile.damping) {}

		float a[STRING_MAX_HARMONICS + 1] = { 0 };
		float b[STRING_MAX_HARMONICS + 1] = { 0 };

		float sampleU(float x) const;
		void update(float t, float dt) override;
		void applyImpulse(float x, float J);
		float getLength() const { return L; }

		Transform transform;

	private:
		float t = 0;
		float L, tension, rho, ESK2, B, c, f0, damping;
		int nHarmonics;
		float harmonicFreqs[STRING_MAX_HARMONICS + 1] = { 0 };
		float harmonicOmega[STRING_MAX_HARMONICS + 1] = { 0 };

		float getHarmonicFreq(int n) const;

	};


	class StringProfile {
	public:
		virtual SingleStringProfile getProfile(int pitch) const = 0;
	};

	class GrandPianoStringProfile : public StringProfile {
	public:

		float calculateStringTension(float frequency, float density, float length, float stiffness) const
		{
			float pi_stiffness_div_L = InstrumentPhysics::PI * stiffness / length;
			return 4 * frequency * frequency * density * length * length - pi_stiffness_div_L * pi_stiffness_div_L;
		}

		SingleStringProfile getProfile(int pitch) const override {
			float frequency = 440 * std::pow(2.0, (pitch - 69) / 12.0);
			float length = 0.657 * std::pow(1.91956, -(pitch - 60) / 12.0);
			float density = 0.02792;
			float stiffness = 2.67e-4;
			float tension = calculateStringTension(frequency, density, length, stiffness);
			float damping = 0.05;
			return { length, tension, density, stiffness, damping, 30 };
		}
	};
}