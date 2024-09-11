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

		virtual void update(float t, float dt) {}; 

		// Some objects may have optimization that requires dt to be known. Called right after the object is added to the simulation and before the first update
		virtual void setDt(float dt) {};
	};
}