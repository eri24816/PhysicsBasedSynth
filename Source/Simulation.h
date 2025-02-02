/*
  ==============================================================================

    Simulation.h
    Created: 26 Aug 2024 12:29:10am
    Author:  a931e

  ==============================================================================
*/

#pragma once
#include <vector>
#include <memory>
#include "Object.h"
#include "Interactions.h"

namespace InstrumentPhysics
{
	class Simulation
	{
	public:
		Simulation(float dt);
		~Simulation();

		void addObject(std::shared_ptr<Object> object);
		void addInteraction(std::shared_ptr<Interaction> interaction);
		float getTime() const;
		void update();
		void setDt(float dt);
	private:
		float t;
		float dt;

		std::vector<std::shared_ptr<Object>> objects;
		std::vector<std::shared_ptr<Interaction>> interactions;
	};
}