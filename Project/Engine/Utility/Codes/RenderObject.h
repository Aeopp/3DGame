#pragma once
#include "Object.h"
#include "RenderInterface.h"
#include "MathStruct.h"

// �ش� Ŭ������ ������ ���õ� ������Ʈ�� ���� ���� ���� �������� �����ϴ� ������Ʈ.
namespace Engine
{
	class DLL_DECL RenderObject : public Object
	{
	public:
		using Super = Object; 
	public:
		void Initialize()&;
		void PrototypeInitialize(IDirect3DDevice9* const Device)&;
	public:
		virtual void Event()&;
		virtual void LateUpdate(const float DeltaTime) & override;
		virtual void Update(const float DeltaTime) & override;
	protected:
		IDirect3DDevice9* Device{ nullptr };
	};
};



