#pragma once

#include "cinder/app/App.h"
#include "cinder/Timeline.h"

class SystemVars
{
public:
	static SystemVars& getInstance()
	{
		static SystemVars    instance;
		// Instantiated on first use.
		return instance;
	}

	//add here variables for the config file

	ci::ivec2   windowSize;
	ci::ivec2	startPosition;

	int			numKinects;

private:

	SystemVars() {

		numKinects = 1;
	};

	SystemVars(SystemVars const&) = delete;
	void operator=(SystemVars const&) = delete;

};