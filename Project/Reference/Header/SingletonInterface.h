#pragma once
#include <memory>

template<typename SubType>
class SingletonInterface
{
protected:
	SingletonInterface() = default;
public:
	friend class std::shared_ptr<SubType>;
	~SingletonInterface()noexcept = default;
	SingletonInterface(const   SingletonInterface&) = delete;
	SingletonInterface(SingletonInterface&&) = delete;
	SingletonInterface&operator=(const  SingletonInterface&) = delete;
	SingletonInterface&operator=(SingletonInterface&&) = delete;
public:
	static SubType& Instance();
};

template<typename SubType>
inline SubType& SingletonInterface<SubType>::Instance()
{
	static std::shared_ptr<SubType> _Instance;

	if (!_Instance)
		_Instance = std::make_shared<SubType>();

	return *_Instance;
}
