#pragma once

#include "Management.h"


inline auto& GetSound() { return Engine::Management::Instance->_Sound; };


//class Sound* _Sound{ nullptr };
//class Timer* _Timer{ nullptr };
//class Controller* _Controller{ nullptr };
//class GraphicDevice* _GraphicDevice{ nullptr };
//class Renderer* _Renderer{ nullptr };