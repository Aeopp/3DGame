#pragma once
#include <memory>

template<typename SubType>
class SingletonInterface
{
protected:
	SingletonInterface() = default;
public:
	friend class std::unique_ptr<SubType>;
	~SingletonInterface()noexcept = default;
	SingletonInterface(const   SingletonInterface&) = delete;
	SingletonInterface(SingletonInterface&&) = delete;
	SingletonInterface& operator=(const  SingletonInterface&) = delete;
	SingletonInterface& operator=(SingletonInterface&&) = delete;
public:
	template<typename... Params>
	static SubType* Init(Params&&... _Params);
	static void Reset();
	// static 변수이기 때문에 DLL 프로젝트에서 사용할시 어떠한 프로세서의 메모리에
	// Instance 가 생성 되는지 유의할 것
	static inline std::unique_ptr<SubType> Instance{ nullptr };
};

template<typename SubType>
void SingletonInterface<SubType>::Reset()
{
	Instance.reset();
};

template<typename SubType>
template<typename... Params>
static SubType* SingletonInterface<SubType>::Init(Params&&... _Params)
{
	Instance = std::make_unique<SubType>();
	Instance->Initialize(std::forward<Params>(_Params)...);
	return Instance.get();
};

