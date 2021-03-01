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
						/*��κ��� ��� ����Ʈ�� ������� ����մϴ�.*/
						const uint32 Width,
						const uint32 Height,
						/*ARGB �� ä�ο� �Ҿ��ϴ� ��Ʈ ������ �˷��ּ���.*/
						/*D3DFMT_A16B16G16R16F  D3DFMT_A32B32G32R32F*/
						const D3DFORMAT Format,
						const D3DXCOLOR Color)&;
		void DepthStencilInitialize(IDirect3DDevice9* Device,
			const uint32 Width,
			const uint32 Height,
			const D3DFORMAT Format);

			// �Լ��� �����ϸ� ����̽��� ���� �ش� �ε��� ����Ÿ���� this 
			// DX9 API�� ���̴����� �ѹ��� �н��� �ִ� 4���� �ȼ����� 
			// ��� �����ϸ� ������ ��ġ�� �ѹ��� ���ε� ������ ����Ÿ�� 
			// �ε��� ������ 0~3�� ���ѵ˴ϴ�. �� ���� ū �ε�����
			// �ǹ̰� �����ϴ�.
		void BindGraphicDevice(const uint32 Index)&;
		void BindDepthStencil()&;

		// Ŭ���� �ϱ����� 0�� �ε����� �����ȴٴ� ���� �����ϼ���.
		void Clear()&;
		void ClearWithDepthStencil(const DWORD Flags)&;

		// ����Ʈ�� �»�� ���� �ؽ��� ���� ��ǥ�� ����Ʈ���� ����� �Է����ּ���.
		void DebugBufferInitialize(const Vector2& ViewPortLeftTopAnchor,
								   const Vector2& Size)&; 
		// ��� ���̴��� ������̺� ���ڿ��� �����Ͽ��ּ���.
		void BindShaderTexture(ID3DXEffect* const Fx,
								const std::string& ConstantTableName)&;

		void RenderDebugBuffer()&;

		IDirect3DTexture9* GetTexture()const& { return TargetTexture;  };

		uint32 Width = 0u;
		uint32 Height = 0u;
	private:
		uint32 Stride = 0u;
		DWORD FVF = 0u;
		const uint32 UniqueResourceID = 0u;
		IDirect3DDevice9* Device{ nullptr };
		// ����Ÿ�ٿ� �ؽ��� .
		IDirect3DTexture9* TargetTexture{ nullptr };
		// ȭ�� ���� �ؽ��� . 
		IDirect3DSurface9* TargetSurface{ nullptr };
		// ���� ���ٽ�
		IDirect3DSurface9* TargetDepthStencil{ nullptr };
		D3DXCOLOR ClearColor{};
		IDirect3DVertexBuffer9* VtxBuf{ nullptr };
		IDirect3DIndexBuffer9* IdxBuf{ nullptr };

	};
};



