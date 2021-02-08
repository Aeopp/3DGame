#include "UtilityGlobal.h"

std::pair<uint32, uint32> Engine::Global::ClientSize{};
HWND Engine::Global::Hwnd;
bool Engine::Global::bDebugMode{ true };
std::filesystem::path Engine::Global::ResourcePath{ L"..\\..\\..\\Resource" };
std::filesystem::path Engine::Global::ResourcePathA{ "..\\..\\..\\Resource" };
Assimp::Importer Engine::Global::AssimpImporter{};

