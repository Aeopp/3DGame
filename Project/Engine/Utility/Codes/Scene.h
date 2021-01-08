#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include <any>
#include <d3d9.h>
#include <unordered_map>
#include "Layer.h"

namespace Engine
{
	class DLL_DECL Scene abstract
	{
	public:
		explicit Scene(IDirect3DDevice9& _Device);
		virtual ~Scene() noexcept = default; 
	public:
		virtual void Initialize()&abstract;
		void Update(const float DeltaTime)&;
		void Render()&;
	private:
		std::unordered_map<std::wstring, std::unique_ptr<Layer>> _LayerMap;
		std::reference_wrapper<IDirect3DDevice9> _Device;
	};
};
