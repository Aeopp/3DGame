#pragma once

#ifdef	ENGINE_EXPORTS
#define DLL_DECL _declspec(dllexport)
#else
#define DLL_DECL _declspec(dllimport)
#endif