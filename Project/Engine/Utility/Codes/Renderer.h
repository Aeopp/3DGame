#pragma once
#include "DllHelper.h"
#include "TypeAlias.h"
#include "SingletonInterface.h"
#include "RenderInterface.h"
#include <map>
#include <vector>
namespace Engine
{
	class DLL_DECL Renderer : public SingletonInterface<Renderer>
	{
	public:
		void Initialize(IDirect3DDevice9* const Device)&;
		void Render()&; 
		void Regist(RenderInterface* const Target);
	private:
		void RenderEnviroment()&;
	private:
		std::map<RenderInterface::Group, std::vector<std::reference_wrapper<RenderInterface>>>
			RenderObjects;
		IDirect3DDevice9* Device{ nullptr };
	};
};



