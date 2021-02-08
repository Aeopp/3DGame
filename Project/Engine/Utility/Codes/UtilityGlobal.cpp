#include "UtilityGlobal.h"

bool Engine::Global::bDebugMode{ true };
std::filesystem::path Engine::Global::ResourcePath{ L"..\\..\\..\\Resource" };
std::filesystem::path Engine::Global::ResourcePathA{ "..\\..\\..\\Resource" };
Assimp::Importer Engine::Global::AssimpImporter{};

