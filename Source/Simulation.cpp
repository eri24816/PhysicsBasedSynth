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

	float Simulation::getTime() const
	{
		return t;
	}

	void Simulation::update(float dt)
	{
		for (auto& interaction : interactions)
		{
			interaction->apply(t, dt);
		}
		for (auto& object : objects)
		{
			object->update(t, dt);
		}
		t += dt;
	}

	void Simulation::addObject(std::shared_ptr<Object> object)
	{
		objects.push_back(object);
	}

	void Simulation::addInteraction(std::shared_ptr<Interaction> interaction)
	{
		interactions.push_back(interaction);
	}
}