#pragma once
#include "Object.h"
#include "RenderInterface.h"

// �ش� Ŭ������ ������ ���õ� ������Ʈ�� ���� ���� ���� �������� �����ϴ� ������Ʈ.
namespace Engine
{
	class DLL_DECL RenderObject : public Object , public RenderInterface
	{
	public:
		using Super = Object; 
	public:
		void Initialize(const RenderInterface::Group _Group)&;
	public:
		virtual void Render()& abstract;
		virtual void LateUpdate(const float DeltaTime) & override;
		virtual void Update(const float DeltaTime) & override;
	};
};



