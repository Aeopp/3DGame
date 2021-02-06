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

//template<typename ObjectSubType,	typename...Params>
//	void LoadPrototype(const std::wstring& Tag,Params&&... _Params)
//{
//		GetProto()->LoadPrototype<ObjectSubType>(Tag,std::forward<Params>(_Params)...);
//};

//class Sound* _Sound{ nullptr };
//class Timer* _Timer{ nullptr };
//class Controller* _Controller{ nullptr };
//class GraphicDevice* _GraphicDevice{ nullptr };
//class Renderer* _Renderer{ nullptr };