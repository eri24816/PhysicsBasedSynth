/*
  ==============================================================================

    String.h
    Created: 25 Aug 2024 10:44:29pm
    Author:  a931e

  ==============================================================================
*/

#pragma once

// use AVX2 intrinsics to speed up the computation. comment out this line if you don't want to use AVX2s
#define USE_AVX2

#include <cmath>
#include "Object.h"
#include "PhysicsUtil.h"

#include <immintrin.h>
#include <vector>
#include <memory>
namespace InstrumentPhysics {

	constexpr float PI = 3.14159265358979323846;
	constexpr float TWO_PI = 2 * PI;
	constexpr int STRING_MAX_HARMONICS = 48;


	struct SingleStringProfile {
		float length, tension, density, stiffness, damping, harmonics;
	};

#ifndef USE_AVX2
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

#endif

#ifdef USE_AVX2
	/*
By sin(x+d) = sin(x)cos(d) + cos(x)sin(d), we can compute sin(x+d) and cos(x+d) from sin(x) and cos(x) with only multiplication and addition.
*/
	class FastSuccessiveSinCos {
	public:
		FastSuccessiveSinCos(__m256 x, __m256 d, float calculateExactInterval=30)
		{
			this->calculateExactInterval = calculateExactInterval;
			this->x = x;
			this->d = d;
			sinX = _mm256_sin_ps(this->x);
			cosX = _mm256_cos_ps(this->x);
			sinD = _mm256_sin_ps(this->d);
			cosD = _mm256_cos_ps(this->d);
			calculateExact();
		}

		inline void next() {
			__m256 oldSinX = sinX;
			sinX = _mm256_add_ps(_mm256_mul_ps(oldSinX, cosD), _mm256_mul_ps(cosX, sinD));
			cosX = _mm256_sub_ps(_mm256_mul_ps(cosX, cosD), _mm256_mul_ps(oldSinX, sinD));
		}


		void calculateExact() {
			sinX = _mm256_sin_ps(this->x);
			cosX = _mm256_cos_ps(this->x);
		}
		__m256 sinX, cosX;
	private:
		float calculateExactInterval, counter = 0;
		__m256 x, d;
		__m256 sinD, cosD;
	};

	class String : public Object
	{
	public:
		String(float L, float tension, float density, float stiffness, int harmonics, float damping);
		String(SingleStringProfile profile) : String(profile.length, profile.tension, profile.density, profile.stiffness, profile.harmonics, profile.damping) {}

		void setDt(float dt) override;
		float sampleU(float x) const;
		void update(float t, float dt) override;
		void applyImpulse(float x, float J);
		float getLength() const { return L; }

		Transform transform;

	private:
		float t = 0;
		float L, tension, rho, ESK2, B, c, f0, damping;
		float recip_L, pi_div_L, two_div_rho_L;
		int nHarmonics;


		__m256 a[STRING_MAX_HARMONICS / 8];
		__m256 b[STRING_MAX_HARMONICS / 8];
		__m256 n[STRING_MAX_HARMONICS / 8];

		__m256 harmonicFreqs[STRING_MAX_HARMONICS / 8];
		__m256 harmonicOmega[STRING_MAX_HARMONICS / 8];

		std::vector<std::unique_ptr<FastSuccessiveSinCos>> omegaTFastSinCos;

		float getHarmonicFreq(int n) const;

	};

#endif

	class StringProfile {
	public:
		virtual SingleStringProfile getProfile(int pitch, float pLength, float pDensity, float pStiffness, float pDamping, float pHarmonics) const = 0;
	};

	class GrandPianoStringProfile : public StringProfile {
	public:

		float calculateStringTension(float frequency, float density, float length, float stiffness) const
		{
			float pi_stiffness_div_L = InstrumentPhysics::PI * stiffness / length;
			return 4 * frequency * frequency * density * length * length - pi_stiffness_div_L * pi_stiffness_div_L;
		}

		/*
		Arguments starts with "p" are parameters that can be adjusted by the user.
		*/
		SingleStringProfile getProfile(int pitch, float pLength, float pDensity, float pStiffness, float pDamping, float pHarmonics) const override{
			float frequency = 440 * std::pow(2.0, (pitch - 69) / 12.0);
			float length = 0.657 * std::pow(1.91956, -(pitch - 60) / 12.0) * pLength;
			float density = 0.02792 * pDensity;
			float stiffness = 2.67e-4 * pStiffness;
			float tension = calculateStringTension(frequency, density, length, stiffness);
			float damping = 0.05 * pDamping;
			return { length, tension, density, stiffness, damping, pHarmonics };
		}
	};
}