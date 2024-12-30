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
	Simulation::Simulation(float dt)
		: dt(dt)
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

	void Simulation::update()
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

	void Simulation::setDt(float dt)
	{
		this->dt = dt;
		for (auto& object : objects)
		{
			object->setDt(dt);
		}
	}

	void Simulation::addObject(std::shared_ptr<Object> object)
	{
		object->setDt(dt);
		objects.push_back(object);
	}

	void Simulation::addInteraction(std::shared_ptr<Interaction> interaction)
	{
		interactions.push_back(interaction);
	}
}