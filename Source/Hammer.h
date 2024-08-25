/*
  ==============================================================================

    Hammer.h
    Created: 25 Aug 2024 11:54:00pm
    Author:  a931e

  ==============================================================================
*/

#pragma once

#include "Object.h"

namespace InstrumentPhysics
{
	class Hammer : public Object
	{
	public:
		Hammer();
		~Hammer();

		void update(float t, float dt) override;
	};
}