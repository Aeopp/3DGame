#pragma once
#include "DllHelper.H"
#include "TypeAlias.h"

namespace Engine
{
	class DLL_DECL RenderInterface abstract
	{
	public:
		enum class Group :uint8
		{
			Enviroment = 0u,
		};
	public:
		virtual void Render()& abstract;
		inline const Group GetGroup()const& { return _Group; };
	protected:
		Group _Group;
	};
};



