#pragma once
#include "TypeAlias.h"
#include "DLLHELPER.H"
#include "SingletonInterface.h"

namespace Engine
{
	class DLL_DECL Shader : public SingletonInterface<Shader>
	{
	public:
		struct Information
		{

		};
	public:
		void Initialize(IDirect3DDevice9* const Device)&;
	public:
					
	private:
		IDirect3DDevice9* Device{ nullptr };
	};
};



