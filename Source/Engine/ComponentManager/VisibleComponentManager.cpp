#include "VisibleComponentManager.h"
#include "../Component/VisibleComponent.h"
#include "../Core/InnoMemory.h"
#include "../Core/InnoLogger.h"
#include "../Common/CommonMacro.inl"
#include "CommonFunctionDefinitionMacro.inl"
#include "../ComponentManager/ITransformComponentManager.h"

#include "../Interface/IModuleManager.h"

extern IModuleManager* g_pModuleManager;

namespace VisibleComponentManagerNS
{
	const size_t m_MaxComponentCount = 32768;
	size_t m_CurrentComponentIndex = 0;
	IObjectPool* m_ComponentPool;
	ThreadSafeVector<VisibleComponent*> m_Components;
	ThreadSafeUnorderedMap<InnoEntity*, VisibleComponent*> m_ComponentsMap;

	std::function<void()> f_SceneLoadingStartCallback;
	std::function<void()> f_SceneLoadingFinishCallback;
	std::function<void(VisibleComponent*, bool)> f_LoadAssetTask;
	std::function<void(VisibleComponent*)> f_AssignUnitMeshTask;
	std::function<void(VisibleComponent*)> f_PDCTask;

	void assignUnitMesh(MeshShapeType meshShapeType, VisibleComponent* visibleComponent)
	{
		visibleComponent->m_modelIndex = g_pModuleManager->getAssetSystem()->addUnitModel(meshShapeType);
	}
}

using namespace VisibleComponentManagerNS;

bool InnoVisibleComponentManager::Setup()
{
	m_ComponentPool = InnoMemory::CreateObjectPool<VisibleComponent>(m_MaxComponentCount);
	m_Components.reserve(m_MaxComponentCount);
	m_ComponentsMap.reserve(m_MaxComponentCount);

	f_SceneLoadingStartCallback = [&]() {
		CleanComponentContainers(VisibleComponent);
	};

	f_SceneLoadingFinishCallback = [&]() {
	};

	g_pModuleManager->getFileSystem()->addSceneLoadingStartCallback(&f_SceneLoadingStartCallback);
	g_pModuleManager->getFileSystem()->addSceneLoadingFinishCallback(&f_SceneLoadingFinishCallback);

	f_LoadAssetTask = [=](VisibleComponent* i, bool AsyncLoad)
	{
		i->m_modelIndex = g_pModuleManager->getFileSystem()->loadModel(i->m_modelFileName.c_str(), AsyncLoad);
	};

	f_AssignUnitMeshTask = [=](VisibleComponent* i)
	{
		assignUnitMesh(i->m_meshShapeType, i);
	};

	f_PDCTask = [=](VisibleComponent* i)
	{
		i->m_PDCIndex.m_startOffset = g_pModuleManager->getPhysicsSystem()->getCurrentPhysicsDataComponentOffset();
		i->m_PDCIndex.m_count = i->m_modelIndex.m_count;

		auto l_transformComponent = GetComponent(TransformComponent, i->m_ParentEntity);
		auto l_globalTm = l_transformComponent->m_globalTransformMatrix.m_transformationMat;

		for (uint64_t j = 0; j < i->m_modelIndex.m_count; j++)
		{
			auto l_meshMaterialPair = g_pModuleManager->getAssetSystem()->getMeshMaterialPair(i->m_modelIndex.m_startOffset + j);

			auto l_PDC = g_pModuleManager->getPhysicsSystem()->generatePhysicsDataComponent(l_meshMaterialPair);
			g_pModuleManager->getPhysicsSystem()->generateAABBInWorldSpace(l_PDC, l_globalTm);
			g_pModuleManager->getPhysicsSystem()->generatePhysicsProxy(i);
		}

		i->m_ObjectStatus = ObjectStatus::Activated;
	};

	return true;
}

bool InnoVisibleComponentManager::Initialize()
{
	return true;
}

bool InnoVisibleComponentManager::Simulate()
{
	return true;
}

bool InnoVisibleComponentManager::Terminate()
{
	return true;
}

InnoComponent * InnoVisibleComponentManager::Spawn(const InnoEntity* parentEntity, ObjectSource objectSource, ObjectOwnership objectUsage)
{
	SpawnComponentImpl(VisibleComponent);
}

void InnoVisibleComponentManager::Destroy(InnoComponent * component)
{
	DestroyComponentImpl(VisibleComponent);
}

InnoComponent* InnoVisibleComponentManager::Find(const InnoEntity * parentEntity)
{
	GetComponentImpl(VisibleComponent, parentEntity);
}

const std::vector<VisibleComponent*>& InnoVisibleComponentManager::GetAllComponents()
{
	return m_Components.getRawData();
}

void InnoVisibleComponentManager::LoadAssetsForComponents(bool AsyncLoad)
{
	for (auto i : m_Components)
	{
		if (i->m_visibilityType != VisibilityType::Invisible)
		{
			if (!i->m_modelFileName.empty())
			{
				if (AsyncLoad)
				{
					auto l_loadAssetTask = g_pModuleManager->getTaskSystem()->submit("LoadAssetTask", 4, nullptr, f_LoadAssetTask, i, true);
					g_pModuleManager->getTaskSystem()->submit("PDCTask", 4, l_loadAssetTask, f_PDCTask, i);
				}
				else
				{
					f_LoadAssetTask(i, false);
					f_PDCTask(i);
				}
			}
			else
			{
				if (i->m_meshShapeType != MeshShapeType::Custom)
				{
					f_AssignUnitMeshTask(i);
					f_PDCTask(i);
				}
				else
				{
					InnoLogger::Log(LogLevel::Warning, "VisibleComponentManager: Custom shape mesh specified without a model preset file.");
				}
			}
		}
	}
}