#pragma once

#include "TypeAlias.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

inline Matrix AssimpTo(const aiMatrix4x4& AiMatrix)
{
	Matrix _Matrix; 
	std::memcpy(&_Matrix, &AiMatrix, sizeof(Matrix));
	return _Matrix;
};

inline Vector3 AssimpTo(const aiVector3D& AiVector3D)
{
	return Vector3
	{
		AiVector3D.x , 
		AiVector3D.y ,
		AiVector3D.z 
	};
};

inline Quaternion AssimpTo(const aiQuaternion& AiQuaternion)
{
	return Quaternion{
	AiQuaternion.x ,
	AiQuaternion.y ,
	AiQuaternion.z ,
	AiQuaternion.w
	};
};







