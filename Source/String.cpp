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


/*
By sin(x+d) = sin(x)cos(d) + cos(x)sin(d), we can compute sin(x+d) and cos(x+d) from sin(x) and cos(x) with only multiplication and addition.
*/
class FastSequentialSinCos {
public:
	FastSequentialSinCos(float x, float d, int calculatePerciseInterval = 100){
		this->x = x;
		this->d = d;
		this->calculatePerciseInterval = calculatePerciseInterval;
		counter = 0;
		sinD = std::sin(d);
		cosD = std::cos(d);
		calculatePercise();
	}

	inline void next() {
		float oldSinX = sinX;
		sinX = sinX * cosD + cosX * sinD;
		cosX = cosX * cosD - oldSinX * sinD;
	}

private:
	float calculatePerciseInterval, counter = 0;
	float x, d;
	float sinX, cosX, sinD, cosD;

	void calculatePercise() {
		sinX = std::sin(x);
		cosX = std::cos(x);
		counter = 0;
	}

};

namespace InstrumentPhysics {
#ifndef USE_AVX2
	String::String(float L, float tension, float rho, float ESK2, int nHarmonics, float damping)
		: L(L), tension(tension), rho(rho), ESK2(ESK2), nHarmonics(nHarmonics), transform(nullptr, Vector2<float>{0.0f, 0.0f}), damping(damping)
	{
		// raise if nHarmonics > STRING_MAX_HARMONICS

		this->nHarmonics = std::min(this->nHarmonics, STRING_MAX_HARMONICS);
		this->nHarmonics = (this->nHarmonics / 8) * 8; // Round down to the nearest multiple of 8 (for vectorization)

		B = PI * PI * ESK2 / tension / L / L;
		c = std::sqrtf(tension / rho);
		f0 = c / (2 * L);

		for (int n = 1; n <= this->nHarmonics; n++)
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
		float pi_x_div_L = PI * x / L;
		for (int n = 1; n <= nHarmonics; n++)
		{
			const float xComp = std::sin(n * pi_x_div_L);
			const float omegaT = harmonicOmega[n] * t;
			u += (a[n] * std::cos(omegaT) + b[n] * std::sin(omegaT)) * xComp;
		}
		return u;
	}



	void String::update(float t, float dt)
	{
		// A String have to store the time to have a complete state
		this->t = t;
		// TODO: use a physical damping model
		if (damping != 0) {
			for (int n = 1; n <= nHarmonics; n++)
			{
				float factor = std::exp(-damping * 0.002 * dt * harmonicFreqs[n]);
				a[n] *= factor;
				b[n] *= factor;
			}
		}
	}

	
	void String::applyImpulse(float x, float J)
	{
		for (int n = 1; n <= nHarmonics; n++)
		{
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

#endif

#ifdef USE_AVX2


	String::String(float L, float tension, float rho, float ESK2, int nHarmonics, float damping)
		: L(L), tension(tension), rho(rho), ESK2(ESK2), nHarmonics(nHarmonics), transform(nullptr, Vector2<float>{0.0f, 0.0f}), damping(damping)
	{
		// raise if nHarmonics > STRING_MAX_HARMONICS

		this->nHarmonics = std::min(this->nHarmonics, STRING_MAX_HARMONICS);
		this->nHarmonics = (this->nHarmonics / 8) * 8; // Round down to the nearest multiple of 8 (for vectorization)

		B = PI * PI * ESK2 / tension / L / L;
		c = std::sqrtf(tension / rho);
		f0 = c / (2 * L);

		//for (int n = 1; n <= this->nHarmonics; n++)
		//{
		//	// precompute stuff for speed
		//	harmonicFreqs[n] = getHarmonicFreq(n);
		//	harmonicOmega[n] = TWO_PI * harmonicFreqs[n];
		//	a[n] = 0;
		//	b[n] = 0;
		//}


		for (int i = 0; i < nHarmonics / 8; i++)
		{

			// Initialize a, b to zero
			a[i] = _mm256_setzero_ps();
			b[i] = _mm256_setzero_ps();
			// Initialize n to 1, 2, 3, ..., 8
			n[i] = _mm256_set_ps(i * 8 + 8, i * 8 + 7, i * 8 + 6, i * 8 + 5, i * 8 + 4, i * 8 + 3, i * 8 + 2, i * 8 + 1);

			float temp[8];
			for (int j = 0; j < 8; j++) {
				int n = i * 8 + j + 1;
				// precompute stuff for speed
				temp[j] = getHarmonicFreq(n);
			}

			harmonicFreqs[i] = _mm256_loadu_ps(temp);
			harmonicOmega[i] = _mm256_mul_ps(_mm256_set1_ps(TWO_PI), harmonicFreqs[i]);
		}

		juce::Logger::writeToLog("String tension: " + juce::String(tension));
		juce::Logger::writeToLog("String density: " + juce::String(rho));
		juce::Logger::writeToLog("String stiffness: " + juce::String(ESK2));
		juce::Logger::writeToLog("String length: " + juce::String(L));
		juce::Logger::writeToLog("String f0: " + juce::String(f0));
	}


	static float horizontal_sum(__m256 vec) {
		/*__m128 low = _mm256_castps256_ps128(vec);
		__m128 high = _mm256_extractf128_ps(vec, 1);
		low = _mm_add_ps(low, high);
		__m128 shuf = _mm_movehdup_ps(low);
		low = _mm_add_ps(low, shuf);
		shuf = _mm_movehl_ps(shuf, low);
		low = _mm_add_ss(low, shuf);
		return _mm_cvtss_f32(low);*/
		vec = _mm256_hadd_ps(vec, vec);
		return vec.m256_f32[0] + vec.m256_f32[1] + vec.m256_f32[4] + vec.m256_f32[5];
	}

	//Vectorized version of sampleU using _mm256_sincos_pd and _mm256_fmadd_pd etc.
	// This function assumes that nHarmonics is a multiple of 8 for simplicity.
	float String::sampleU(float x) const
	{
		// Initialize accumulators to zero
		__m256 u_vec = _mm256_setzero_ps();

		// Prepare constant values
		const float pi_x_div_L = PI * x / L;
		__m256 pi_x_div_L_vec = _mm256_set1_ps(pi_x_div_L);

		for (int i = 0; i < nHarmonics/8; i++)
		{

			// Compute n * pi_x_div_L for the sine term
			__m256 n_pi_x_div_L_vec = _mm256_mul_ps(n[i], pi_x_div_L_vec);

			// Compute sin(n * pi_x_div_L)
			__m256 xComp_vec = _mm256_sin_ps(n_pi_x_div_L_vec);

			// Compute omegaT = harmonicOmega[n] * t
			__m256 t_vec = _mm256_set1_ps(t);
			__m256 omegaT_vec = _mm256_mul_ps(harmonicOmega[i], t_vec);

			// Compute cos(omegaT) and sin(omegaT)
			__m256 sinOmegaT_vec, cosOmegaT_vec;
			sinOmegaT_vec = _mm256_sincos_ps(&cosOmegaT_vec, omegaT_vec);

			// (a[n] * cos(omegaT) + b[n] * sin(omegaT)) * sin(n * pi_x_div_L)
			__m256 term1 = _mm256_mul_ps(a[i], cosOmegaT_vec);
			__m256 term2 = _mm256_mul_ps(b[i], sinOmegaT_vec);
			__m256 result_vec = _mm256_add_ps(term1, term2);

			// Multiply by xComp
			result_vec = _mm256_mul_ps(result_vec, xComp_vec);

			// Accumulate result in u_vec
			u_vec = _mm256_add_ps(u_vec, result_vec);
		}

		// Horizontal sum to reduce u_vec to a single float
		float u = horizontal_sum(u_vec);

		return u;
	}



	void String::update(float t, float dt)
	{
		// A String have to store the time to have a complete state
		this->t = t;
		// TODO: use a physical damping model
		if (damping != 0) {
			__m256 factorConst = _mm256_set1_ps(-damping * 0.002 * dt);
			for (int i = 0; i < nHarmonics / 8; i++)
			{
				__m256 factor = _mm256_mul_ps(factorConst, harmonicFreqs[i]);
				factor = _mm256_exp_ps(factor);
				a[i] = _mm256_mul_ps(a[i], factor);
				b[i] = _mm256_mul_ps(b[i], factor);
			}
		}
	}


	// Vectorized version of applyImpulse

	void String::applyImpulse(float x, float J)
	{
		// Precompute constant factor
		const float factor = 2 * J / L / rho;
		__m256 factor_vec = _mm256_set1_ps(factor);
		__m256 pi_x_div_L_vec = _mm256_set1_ps(PI * x / L);

		for (int i = 0; i < nHarmonics / 8; i++)
		{

			// Compute xComp = (2 * J / L / rho) / harmonicOmega[n] * sin(n * PI * x / L)
			__m256 n_pi_x_div_L_vec = _mm256_mul_ps(n[i], pi_x_div_L_vec);
			__m256 xComp_vec = _mm256_div_ps(factor_vec, harmonicOmega[i]);
			__m256 sin_n_pi_x_div_L_vec = _mm256_sin_ps(n_pi_x_div_L_vec);  // Vectorized sin computation
			xComp_vec = _mm256_mul_ps(xComp_vec, sin_n_pi_x_div_L_vec);

			// Compute omegaT = harmonicOmega[n] * t
			__m256 t_vec = _mm256_set1_ps(t);
			__m256 omegaT_vec = _mm256_mul_ps(harmonicOmega[i], t_vec);

			// Compute sin(omegaT) and cos(omegaT)
			__m256 sinOmegaT_vec, cosOmegaT_vec;
			sinOmegaT_vec = _mm256_sincos_ps(&cosOmegaT_vec, omegaT_vec);

			// a[n] += xComp * cos(omegaT)
			__m256 a_update = _mm256_mul_ps(xComp_vec, cosOmegaT_vec);
			a[i] = _mm256_add_ps(a[i], a_update);

			// b[n] += xComp * sin(omegaT)
			__m256 b_update = _mm256_mul_ps(xComp_vec, sinOmegaT_vec);
			b[i] = _mm256_add_ps(b[i], b_update);
		}
	}


	float String::getHarmonicFreq(int n) const
	{
		return n * f0 * std::sqrtf(1 + B * n * n);
	}
#endif
}