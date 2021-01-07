#pragma once
#include <functional>
#include <memory>
#include <type_traits>
#include <d3d9.h>

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

		return std::shared_ptr<D3DInstanceType>(Ptr, [](auto Ptr)
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
};
