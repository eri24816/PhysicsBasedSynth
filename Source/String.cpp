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

		recip_L = 1.0f / L;
		pi_div_L = PI / L;
		two_div_rho_L = 2.0f / rho / L;

		this->nHarmonics = std::min(this->nHarmonics, STRING_MAX_HARMONICS);
		this->nHarmonics = (this->nHarmonics / 8) * 8; // Round down to the nearest multiple of 8 (for vectorization)

		B = PI * PI * ESK2 / tension / L / L;
		c = std::sqrtf(tension / rho);
		f0 = c / (2 * L);


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
				temp[j] = getHarmonicFreq(n);
			}

			harmonicFreqs[i] = _mm256_loadu_ps(temp);
			harmonicOmega[i] = _mm256_mul_ps(_mm256_set1_ps(TWO_PI), harmonicFreqs[i]);

			// k = n * pi / L
			k[i] = _mm256_mul_ps(n[i], _mm256_set1_ps(pi_div_L));
			half_k_sq[i] = _mm256_mul_ps(_mm256_mul_ps(k[i], k[i]), _mm256_set1_ps(0.5f));
		}

	}


	static float horizontal_sum(__m256 vec) {
		vec = _mm256_hadd_ps(vec, vec);
		return vec.m256_f32[0] + vec.m256_f32[1] + vec.m256_f32[4] + vec.m256_f32[5];
	}

	void String::setDt(float dt){
		omegaTFastSinCos.clear();
		for (int i = 0; i < nHarmonics / 8; i++)
		{
			FastSuccessiveSinCos* fastSinCos = new FastSuccessiveSinCos(_mm256_mul_ps(harmonicOmega[i], _mm256_set1_ps(t)),_mm256_mul_ps(harmonicOmega[i], _mm256_set1_ps(dt)));
			omegaTFastSinCos.push_back(std::unique_ptr<FastSuccessiveSinCos>(fastSinCos));
		}
	}

	//Vectorized version of sampleU using _mm256_sincos_pd and _mm256_fmadd_pd etc.
	// This function assumes that nHarmonics is a multiple of 8 for simplicity.
	float String::sampleU(float x) const
	{
		// Initialize accumulators to zero
		__m256 u_vec = _mm256_setzero_ps();

		// Prepare constant values
		const float pi_x_div_L = pi_div_L * x;
		__m256 pi_x_div_L_vec = _mm256_set1_ps(pi_x_div_L);

		for (int i = 0; i < nHarmonics/8; i++)
		{

			// Compute n * pi_x_div_L for the sine term
			__m256 n_pi_x_div_L_vec = _mm256_mul_ps(n[i], pi_x_div_L_vec);

			// Compute sin(n * pi_x_div_L)
			__m256 xComp_vec = _mm256_sin_ps(n_pi_x_div_L_vec);

			// Compute omegaT = harmonicOmega[n] * t

			__m256 term1 = _mm256_mul_ps(a[i], omegaTFastSinCos[i]->cosX);
			__m256 term2 = _mm256_mul_ps(b[i], omegaTFastSinCos[i]->sinX);

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

		// limit the magnitude of a and b to prevent numerical instability
		// limit: 1.5cm
		__m256 limit = _mm256_set1_ps(0.015f);
		__m256 neg_limit = _mm256_set1_ps(-0.015f);

		for (int i = 0; i < nHarmonics / 8; i++) {
			a[i] = _mm256_min_ps(a[i], limit);
			a[i] = _mm256_max_ps(a[i], neg_limit);
			b[i] = _mm256_min_ps(b[i], limit);
			b[i] = _mm256_max_ps(b[i], neg_limit);
		}

		for (auto& fastSinCos : omegaTFastSinCos) {
			fastSinCos->next();
		}
	}


	// Vectorized version of applyImpulse

	void String::applyImpulse(float x, float J, float sigma=0)
	{
		// Precompute constant factor
		__m256 factor_vec = _mm256_set1_ps(J * two_div_rho_L);
		__m256 x_vec = _mm256_set1_ps(x);

		__m256 neg_sigma_sq_vec; 
		if (sigma != 0) {
			neg_sigma_sq_vec = _mm256_set1_ps(-sigma * sigma);
		}

		for (int i = 0; i < nHarmonics / 8; i++)
		{

			// Compute xComp = (2 * J / L / rho) / harmonicOmega[n] * sin(kx)
			__m256 kx_vec = _mm256_mul_ps(k[i], x_vec);
			__m256 xComp_vec = _mm256_div_ps(factor_vec, harmonicOmega[i]);
			__m256 sin_kx_vec = _mm256_sin_ps(kx_vec);  // Vectorized sin computation
			xComp_vec = _mm256_mul_ps(xComp_vec, sin_kx_vec);

			// if sigma != 0, use gaussian impulse e^{-\frac{1}{2}k^2\sigma^2}

			if (sigma != 0) {
				__m256 gaussian_factor = _mm256_exp_ps(_mm256_mul_ps(half_k_sq[i], neg_sigma_sq_vec));
				xComp_vec = _mm256_mul_ps(xComp_vec, gaussian_factor);
			}

			// a[n] += xComp * cos(omegaT)
			__m256 a_update = _mm256_mul_ps(xComp_vec, omegaTFastSinCos[i]->cosX);
			a[i] = _mm256_add_ps(a[i], a_update);

			// b[n] += xComp * sin(omegaT)
			__m256 b_update = _mm256_mul_ps(xComp_vec, omegaTFastSinCos[i]->sinX);
			b[i] = _mm256_add_ps(b[i], b_update);
		}
	}


	float String::getHarmonicFreq(int n) const
	{
		return n * f0 * std::sqrtf(1 + B * n * n);
	}
#endif
}