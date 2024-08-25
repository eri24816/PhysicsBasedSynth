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

namespace InstrumentPhysics
{
	class Simulation
	{
	public:
		Simulation();
		~Simulation();

		void addObject(std::shared_ptr<Object> object);
		void update(float dt);
	private:
		float t;

		std::vector<std::shared_ptr<Object>> objects;
	};
}