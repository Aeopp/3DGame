#pragma once
#include "DxHelper.h"
#include "TypeAlias.h"
#include "DllHelper.H"
#include <unordered_map>
#include <string>
#include <map>
#include <optional>

namespace Engine
{
	class DLL_DECL RenderTarget
	{
	public :
		RenderTarget();
		RenderTarget(const RenderTarget&) = default;
		RenderTarget(RenderTarget&&) noexcept= default;
		RenderTarget& operator=(const RenderTarget&) = default;
		RenderTarget& operator=(RenderTarget&&) noexcept = default;
		~RenderTarget()noexcept = default;
		void Initialize(IDirect3DDevice9* const Device, 
						/*대부분의 경우 뷰포트의 사이즈로 충분합니다.*/
						const uint32 Width,
						const uint32 Height,
						/*ARGB 각 채널에 할애하는 비트 정보를 알려주세요.*/
						/*D3DFMT_A16B16G16R16F  D3DFMT_A32B32G32R32F*/
						const D3DFORMAT Format,
						const D3DXCOLOR Color)&;
			 // 함수가 성공하면 디바이스에 현재 렌더타겟은 this 
			 // 이전에 설정되어있던 렌더타겟은 overwirte 되며 때문에 
			// 이전 렌더타겟	정보를 저장합니다. 

			// DX9 API는 쉐이더에서 한번의 패스로 최대 4개의 픽셀까지 
			// 출력 가능하며 때문에 장치에 한번에 바인딩 가능한 렌더타겟 
			// 인덱스 범위도 0~3로 제한됩니다. 이 보다 큰 인덱스는
			// 의미가 없습니다.
		void BindGraphicDevice(const uint32 Index)&;
		//	// 디바이스에 설정된 this 렌더타겟을 해제하며
		//	// 캐싱한 이전 렌더타겟으로 다시 등록합니다.
		//void UnBindGraphicDevice(const uint32 Index)&;
		void Clear()&;

		// 뷰포트의 좌상단 기준 텍스쳐 시작 좌표와 뷰포트기준 사이즈를 입력해주세요.
		void DebugBufferInitialize(const Vector2& ViewPortLeftTopAnchor,
								   const Vector2& Size)&; 
		// 사용 쉐이더와 상수테이블 문자열을 매핑하여주세요.
		void BindShaderTexture(ID3DXEffect* const Fx,
						const std::string& ConstantTableName)&;

		void RenderDebugBuffer()&;

		IDirect3DTexture9* GetTexture()const& { return TargetTexture;  };
	private:
		uint32 Stride = 0u;
		DWORD FVF = 0u;
		const uint32 UniqueResourceID = 0u;
		IDirect3DDevice9* Device{ nullptr };
		// 렌더타겟용 텍스쳐 .
		IDirect3DTexture9* TargetTexture{ nullptr };
		// 화면 단위 텍스쳐 . 
		IDirect3DSurface9* TargetSurface{ nullptr };
		// IDirect3DSurface9* PrevTargetSurface{ nullptr };
		D3DXCOLOR ClearColor{};
		IDirect3DVertexBuffer9* VtxBuf{ nullptr };
		IDirect3DIndexBuffer9* IdxBuf{ nullptr };

	};
};



