/*
  ==============================================================================

    Object.h
    Created: 26 Aug 2024 1:08:40am
    Author:  a931e

  ==============================================================================
*/

#pragma once

namespace InstrumentPhysics
{
	class Object
	{
	public:

		virtual void update(float t, float dt) = 0;
	};
}