#pragma once
#include "Layer.h"

class EnemyLayer : public Engine::Layer
{
public:
	using Super = Engine::Layer;
public:
	void Initialize();
	int Test = 77;
private:
	
};

