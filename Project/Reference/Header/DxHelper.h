#pragma once
#include <functional>
#include <memory>
#include <type_traits>
#include <d3d9.h>
#include <type_traits>
#include "TypeAlias.h"

namespace DX
{
	template<typename D3DInstanceType>
	using SharedPtr = std::shared_ptr<D3DInstanceType>;

	template<typename D3DInstanceType>
	using UniquePtr = std::unique_ptr<D3DInstanceType,
		std::function<void(D3DInstanceType* const)>>;

	template<typename D3DInstanceType>
	std::shared_ptr<D3DInstanceType> MakeShared(D3DInstanceType* const Ptr)
	{
		// 알맞는 D3D 객체 인지 컴파일 타임 검사.
		static_assert(std::is_base_of_v<IUnknown, D3DInstanceType>,
			"There is a suitable D3D instance type.");

		if (!Ptr)
		{
			MessageBox(nullptr, L"NULL PTR", L"Warning!", MB_OK);
		}

		return std::shared_ptr<D3DInstanceType>(Ptr, [](D3DInstanceType* Ptr)
			{
				Ptr->Release();
			});
	};

	template<typename D3DInstanceType>
	auto MakeUnique(D3DInstanceType* const Ptr)
	{
		// 알맞는 D3D 객체 인지 컴파일 타임 검사.
		static_assert(std::is_base_of_v<IUnknown, D3DInstanceType>,
			"There is a suitable D3D instance type.");

		if (!Ptr)
		{
			MessageBox(nullptr, L"NULL PTR", L"Warning!", MB_OK);
		}

		return UniquePtr<D3DInstanceType>
			(Ptr, [](D3DInstanceType* const Ptr)
				{
					Ptr->Release();
				});
	};

	enum class CreateVertexFlag : uint8
	{
		WriteOnly=0u,
		Dynamic,
	};

	template<typename VertexType>
	void CreateVertex(
		IDirect3DDevice9* const Device,
		const std::vector<VertexType>& VertexArray,
		const CreateVertexFlag _CreateVertexFlag,
		uint32& VertexCount/*Out*/,
		uint32& TriangleCount/*Out*/,
		uint16& VertexByteSize/*Out*/,
		std::shared_ptr<IDirect3DVertexBuffer9>& VertexBuffer,/*Out*/
		std::shared_ptr<IDirect3DVertexDeclaration9>& VertexDecl/*Out*/
	) noexcept(false)
	{
		VertexCount = VertexArray.size();
		TriangleCount = VertexCount / 3u;
		VertexByteSize = sizeof(VertexType);

		IDirect3DVertexBuffer9* VertexBufferTemp{ nullptr };

		int32 UsageOption;
		D3DPOOL PoolOption;

		switch (_CreateVertexFlag)
		{
		case CreateVertexFlag::Dynamic:
			UsageOption = D3DUSAGE_DYNAMIC;
			PoolOption = D3DPOOL_DEFAULT;
			break;
		case CreateVertexFlag::WriteOnly:
		default:
			UsageOption = D3DUSAGE_WRITEONLY;
			PoolOption = D3DPOOL_MANAGED;
			break;
		}

		if (FAILED(Device->CreateVertexBuffer(VertexByteSize * VertexCount,
			UsageOption, NULL, PoolOption,
			&VertexBufferTemp, 0)))
			throw std::exception(__FUNCTION__);

		VertexType* VertexPtr;
		VertexBuffer->Lock(0, VertexByteSize * VertexCount, (void**)&VertexPtr,0);
		memcpy(VertexPtr, VertexArray.data(), VertexByteSize * VertexCount);
		VertexBuffer->Unlock();

		VertexDecl = MakeShared(VertexType::GetVertexDecl(Device));
		VertexBuffer = MakeShared(VertexType::GetVertexDecl(Device));
	};
};
