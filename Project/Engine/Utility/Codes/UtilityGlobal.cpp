#include "UtilityGlobal.h"
bool Engine::Global::bDebugMode{ true };
std::filesystem::path Engine::Global::ResourcePath{ L"..\\..\\..\\Resource" };
Assimp::Importer Engine::Global::AssimpImporter{};

