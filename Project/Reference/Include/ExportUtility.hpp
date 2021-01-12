#pragma once

#include "Management.h"

inline auto& GetSound() { return *Engine::Management::Instance->_Sound; };
inline auto& GetManager() { return *Engine::Management::Instance; };
inline auto& GetGraphic() { return *Engine::Management::Instance->_GraphicDevice; };
inline auto& GetController() { return *Engine::Management::Instance->_Controller; };




//class Sound* _Sound{ nullptr };
//class Timer* _Timer{ nullptr };
//class Controller* _Controller{ nullptr };
//class GraphicDevice* _GraphicDevice{ nullptr };
//class Renderer* _Renderer{ nullptr };