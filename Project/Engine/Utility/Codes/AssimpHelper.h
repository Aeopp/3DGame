#pragma once

#include "TypeAlias.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

inline void AssimpDebugPrint(const aiMatrix4x4& AiMatrix)
{
	for (uint32 Row = 0u; Row < 4u; ++Row)
	{
		for (uint32 Col = 0u; Col < 4u; ++Col)
		{
			std::cout << AiMatrix[Row][Col] << " "; 
		}
		std::cout << std::endl; 
	}
	std::cout << std::endl; 
}

// 열기준 행렬을 전치해 DX의 행기준 행렬로 변환한다.
inline Matrix FromAssimp(const aiMatrix4x4& AiMatrix)
{
	Matrix _Matrix;
	std::memcpy(&_Matrix, &AiMatrix, sizeof(aiMatrix4x4));
	D3DXMatrixTranspose(&_Matrix, &_Matrix);
	return _Matrix;
};

inline Vector3 FromAssimp(const aiVector3D& AiVector3D)
{
	return Vector3
	{
		AiVector3D.x , 
		AiVector3D.y ,
		AiVector3D.z 
	};
};

inline Quaternion FromAssimp(const aiQuaternion& AiQuaternion)
{
	return Quaternion{
	AiQuaternion.x ,
	AiQuaternion.y ,
	AiQuaternion.z ,
	AiQuaternion.w
	};
};







