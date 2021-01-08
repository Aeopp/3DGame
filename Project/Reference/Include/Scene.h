#pragma once
#include "DllHelper.h"
#include "SingletonInterface.h"
#include <any>
#include <d3d9.h>

namespace Engine
{
	class DLL_DECL Scene abstract
	{
	public:
		explicit Scene(IDirect3DDevice9& _Device);
	public:
		virtual void Initialize()&abstract;
		void Update(const float DeltaTime)&;
		void Render()&;
	private:
		std::reference_wrapper<IDirect3DDevice9> _Device;
	};
};
