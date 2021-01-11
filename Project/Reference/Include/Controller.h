#pragma once
#include "DLLHELPER.H"
#include "SingletonInterface.h"

class DLL_DECL Controller : public SingletonInterface< Controller >
{
public:
	void foo();
};
	