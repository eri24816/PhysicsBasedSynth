/*
  ==============================================================================

    Simulation.cpp
    Created: 26 Aug 2024 12:29:10am
    Author:  a931e

  ==============================================================================
*/

#include "Simulation.h"

namespace InstrumentPhysics
{
	Simulation::Simulation()
	{
		t = 0.0f;
	}

	Simulation::~Simulation()
	{
	}

	void Simulation::update(float dt)
	{
		t += dt;
	}

	void Simulation::addObject(std::shared_ptr<Object> object)
	{
		objects.push_back(object);
	}
}