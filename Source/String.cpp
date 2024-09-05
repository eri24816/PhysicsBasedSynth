/*
  ==============================================================================

    String.cpp
    Created: 25 Aug 2024 10:47:33pm
    Author:  a931e

  ==============================================================================
*/

#include "String.h"
#include "JuceHeader.h"
#include <cmath>
#include <stdexcept>

namespace InstrumentPhysics {

	String::String(float L, float tension, float rho, float ESK2, int nHarmonics, float damping)
		: L(L), tension(tension), rho(rho), ESK2(ESK2), nHarmonics(nHarmonics), transform(nullptr, Vector2<float>{0.0f, 0.0f}), damping(damping)
	{
		// raise if nHarmonics > STRING_MAX_HARMONICS

		if (nHarmonics > STRING_MAX_HARMONICS)
		{
			throw std::invalid_argument("nHarmonics > STRING_MAX_HARMONICS");
		}

		B = PI * PI * ESK2 / tension / L / L;
		c = std::sqrtf(tension / rho);
		f0 = c / (2 * L);

		for (int n = 1; n <= nHarmonics; n++)
		{
			// precompute stuff for speed
			harmonicFreqs[n] = getHarmonicFreq(n);
			harmonicOmega[n] = TWO_PI * harmonicFreqs[n];
			a[n] = 0;
			b[n] = 0;
		}


		juce::Logger::writeToLog("String tension: " + juce::String(tension));
		juce::Logger::writeToLog("String density: " + juce::String(rho));
		juce::Logger::writeToLog("String stiffness: " + juce::String(ESK2));
		juce::Logger::writeToLog("String length: " + juce::String(L));
		juce::Logger::writeToLog("String f0: " + juce::String(f0));
	}

	float String::sampleU(float x) const
	{
		float u = 0;
		for (int n = 1; n <= nHarmonics; n++)
		{
			const float xComp = std::sin(n * PI * x / L);
			const float omegaT = harmonicOmega[n] * t;
			u += a[n] * std::cos(omegaT) * xComp + b[n] * std::sin(omegaT) * xComp;
		}
		return u;
	}

	void String::update(float t, float dt)
	{
		// A String have to store the time to have a complete state
		this->t = t;
		// TODO: use a physical damping model
		if (damping!=0) {
			for (int n = 1; n <= nHarmonics; n++)
			{
				a[n] *= std::exp(-damping*0.002 * dt * harmonicFreqs[n]);
				b[n] *= std::exp(-damping*0.002 * dt * harmonicFreqs[n]);
			}
		}
	}

	void String::applyImpulse(float x, float J)
	{
		for (int n = 1; n <= nHarmonics; n++)
		{
			a[n] = 0;
			b[n] = 0;
			const float xComp = 2 * J / L / rho / harmonicOmega[n] * std::sin(n * PI * x / L);
			const float omegaT = harmonicOmega[n] * t;

			a[n] += xComp * std::cos(omegaT);
			b[n] += xComp * std::sin(omegaT);
		}
	}

	float String::getHarmonicFreq(int n) const
	{
		return n * f0 * std::sqrtf(1 + B * n * n);
	}
}