/*
  ==============================================================================

    String.h
    Created: 25 Aug 2024 10:44:29pm
    Author:  a931e

  ==============================================================================
*/

#pragma once

#include "Object.h"
#include "PhysicsUtil.h"

namespace InstrumentPhysics {

	constexpr float PI = 3.14159265358979323846;
	constexpr float TWO_PI = 2 * PI;
	constexpr int STRING_MAX_HARMONICS = 50;

	class String : public Object
	{
	public:
		String(float L, float tension, float mass, float ESK2, int nHarmonics);
		~String() = default;

		float a[STRING_MAX_HARMONICS + 1] = { 0 };
		float b[STRING_MAX_HARMONICS + 1] = { 0 };

		float sampleU(float x, float t) const;
		void applyImpulse(float x, float t, float J);

		Transform transform;

	private:
		float L, tension, mass, rho, ESK2, B, c, f0;
		int nHarmonics;
		float harmonicFreqs[STRING_MAX_HARMONICS + 1] = { 0 };
		float harmonicOmega[STRING_MAX_HARMONICS + 1] = { 0 };

		float getHarmonicFreq(int n) const;

	};
}