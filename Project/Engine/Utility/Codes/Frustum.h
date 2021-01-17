#pragma once
#include <array>
#include "TypeAlias.h"
#include "DllHelper.H"
#include "MathStruct.h"

namespace Engine
{
	class DLL_DECL Frustum
	{
	public:
		void Initialize()&;
		~Frustum()noexcept;
	public:
		void Make(const Matrix& CameraWorld, const Matrix& Projection)&;
		bool IsIn(const Vector3& Point)&;
		bool IsIn(const Sphere& _Sphere)&;
		void Render(IDirect3DDevice9* const Device)&;
	private:
		Matrix World{};
		IDirect3DVertexBuffer9 *CubeVtxBuf{ nullptr };
		IDirect3DIndexBuffer9* CubeIdxBuf{ nullptr };
		std::array<D3DXPLANE, 6u> Planes;
		std::array<Vector3, 8u> Points;
	};
}


