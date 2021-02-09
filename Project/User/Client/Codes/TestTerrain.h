#pragma once
#include "RenderObject.h"
#include "ShaderFx.h"

class TestTerrain final: public Engine::RenderObject
{
public:
	using Super = Engine::RenderObject;
public:
	void Initialize(const Vector3& Scale,
					const Vector3& Rotation,
					const Vector3& SpawnLocation)&;
	void PrototypeInitialize(IDirect3DDevice9*const Device ,
							const Engine::RenderInterface::Group _Group)&;
	virtual void Event()&override;
	virtual void Render() & override;
	virtual void Update(const float DeltaTime)&;
private:
	float SunPower{ 100.f };
	Vector4 Sun{ 0,0,10,1.f };
	IDirect3DDevice9* Device{ nullptr };
	Engine::ShaderFx* TestTerrainFx{ nullptr };
};

