#pragma once
#include "RenderObject.h"

class Decorator final : public Engine::RenderObject
{
public:
	using Super = Engine::RenderObject;
public:
	void Initialize(const Vector3& Scale,
		const Vector3& Rotation,
		const Vector3& SpawnLocation,
		const std::wstring MeshName,
		const bool bBoundingBox)&;
	void PrototypeInitialize(IDirect3DDevice9* const Device,
		const Engine::RenderInterface::Group _Group)&;
	virtual void Event() & override;
	virtual void Render() & override;
	virtual void Update(const float DeltaTime)&;
private:
	IDirect3DDevice9* Device{ nullptr };
};

