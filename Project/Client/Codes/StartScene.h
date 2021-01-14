#pragma once
#include "Scene.h"
#include <type_traits>
#include <numeric>

class StartScene final : public Engine::Scene
{
public:
	using Super = Engine::Scene;
public:
	virtual void Initialize(IDirect3DDevice9* const Device)&;
	virtual void Update(const float DeltaTime) & override;
private:
	
};

template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(T x, T y)
{
	return std::fabs(x - y) <= std::numeric_limits<T>::epsilon() * std::fabs(x + y) * 1

		|| std::fabs(x - y) < (std::numeric_limits<T>::min)();
}


