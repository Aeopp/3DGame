#pragma once

#include "Management.h"

inline auto& RefSound() { return *Engine::Management::Instance->_Sound; };
inline auto& RefManager() { return *Engine::Management::Instance; };
inline auto& RefGraphic() { return *Engine::Management::Instance->_GraphicDevice; };
inline auto& RefControl() { return *Engine::Management::Instance->_Controller; };
inline auto& RefProto() { return *Engine::Management::Instance->_PrototypeManager; };
inline auto& RefFontManager() {return *Engine::Management::Instance->_FontManager;}
inline auto& RefResourceSys() { return *Engine::Management::Instance->_ResourceSys; }
inline auto& RefShaderSys() { return *Engine::Management::Instance->_ShaderManager; };
inline auto& RefNaviMesh() { return *Engine::Management::Instance->_NaviMesh; };
inline auto& RefRenderer() { return *Engine::Management::Instance->_Renderer; };
inline auto& RefTimer() { return *Engine::Management::Instance->_Timer; };

