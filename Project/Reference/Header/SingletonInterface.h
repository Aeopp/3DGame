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
	static  SubType* Init(Params&&... _Params);
	static inline std::unique_ptr<SubType> Instance{ nullptr };
};

template<typename SubType>
template<typename... Params>
static SubType* SingletonInterface<SubType>::Init(Params&&... _Params)
{
	Instance = std::make_unique<SubType>();
	Instance->Initialize(std::forward<Params>(_Params)...);
	return Instance.get();
};

