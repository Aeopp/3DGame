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
			NoAlpha,
			AlphaTest,
			AlphaBlend,
			Particle,
			DebugCollision,
			UI,
		};
	public:
		//   �ʱ�ȭ �ܰ迡�� ������ �Ѱ��ּ���.
		void SetUpRenderingInformation(const Group _Group);
		//   �������� ���ϴ� Ÿ�̹��� ������Ʈ �����ӿ��� ȣ�����ּ���.
		void Regist();
		virtual void Render()& abstract;
		inline const Group GetGroup()const& { return _Group; };
	private:
		Group _Group;
	};
};



