#pragma once

#include "Management.h"

inline auto& GetSound() { return *Engine::Management::Instance->_Sound; };
inline auto& GetManager() { return *Engine::Management::Instance; };
inline auto& GetGraphic() { return *Engine::Management::Instance->_GraphicDevice; };
inline auto& GetControl() { return *Engine::Management::Instance->_Controller; };
inline auto& GetProto() { return *Engine::Management::Instance->_PrototypeManager; };
inline auto& GetFontManager() {return *Engine::Management::Instance->_FontManager;}
//
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