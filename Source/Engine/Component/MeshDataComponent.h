#pragma once
#include "../Common/InnoType.h"
#include "../Common/InnoGraphicsPrimitive.h"
#include "../Common/InnoComponent.h"
#include "SkeletonDataComponent.h"

class MeshDataComponent : public InnoComponent
{
public:
	MeshPrimitiveTopology m_meshPrimitiveTopology = MeshPrimitiveTopology::Triangle;
	MeshSource m_meshSource = MeshSource::Procedural;
	ProceduralMeshShape m_proceduralMeshShape = ProceduralMeshShape::Triangle;
	size_t m_indicesSize = 0;
	SkeletonDataComponent* m_SDC = 0;
	Array<Vertex> m_vertices;
	Array<Index> m_indices;
};
