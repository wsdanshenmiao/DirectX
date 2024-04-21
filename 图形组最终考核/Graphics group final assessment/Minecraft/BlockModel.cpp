#include "BlockModel.h"

using namespace DirectX;

namespace DSM {

Model* BlockModel::GetBedRockModel(TextureManager& textureManager, ModelManager& modelManager)
{
    Model* pModel = modelManager.CreateFromGeometry("BedRock", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
    textureManager.CreateFromFile("..\\Texture\\block\\bedrock.jpg");
    pModel->SetDebugObjectName("BedRock");
    pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\block\\bedrock.jpg");
    pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
    pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
    return pModel;
}

Model* BlockModel::GetDirtModel(TextureManager& textureManager, ModelManager& modelManager)
{
    Model* pModel = modelManager.CreateFromGeometry("Dirt", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
    textureManager.CreateFromFile("..\\Texture\\block\\dirt.jpg");
    pModel->SetDebugObjectName("Dirt");
    pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\block\\dirt.jpg");
    pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
    pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
    return pModel;
}

Model* BlockModel::GetStoneModel(TextureManager& textureManager, ModelManager& modelManager)
{
    Model* pModel = modelManager.CreateFromGeometry("Stone", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
    textureManager.CreateFromFile("..\\Texture\\block\\stone.jpg");
    pModel->SetDebugObjectName("Stone");
    pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\block\\stone.jpg");
    pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
    pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
    return pModel;
}

Model* BlockModel::GetGressModel(TextureManager& textureManager, ModelManager& modelManager)
{
    Model* pModel = modelManager.CreateFromGeometry("Gress", Geometry::CreateBox(1.0f, 1.0f, 1.0f));
    textureManager.CreateFromFile("..\\Texture\\block\\gress0.png");
    pModel->SetDebugObjectName("Gress");
    pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\block\\gress0.png");
    pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
    pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
    return pModel;
}

Model* BlockModel::GetCherryBlossomLogModel(TextureManager& tManager, ModelManager& mManager)
{
    Model* pModel = mManager.CreateFromGeometry("CherryBlossomLog", Geometry::CreateBox(1.0f, 3.0f, 1.0f));
    tManager.CreateFromFile("..\\Texture\\block\\cherryblossomlogs.png");
    pModel->SetDebugObjectName("CherryBlossomLog");
    pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\block\\cherryblossomlogs.png");
    pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
    pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
    return pModel;
}

Model* BlockModel::GetCherryBlossomLeavesModel(TextureManager& tManager, ModelManager& mManager)
{
    Model* pModel = mManager.CreateFromGeometry("CherryBlossomLeaves", Geometry::CreateBox(5.0f, 5.0f, 5.0f));
    tManager.CreateFromFile("..\\Texture\\block\\cherryblossomleaves.png");
    pModel->SetDebugObjectName("CherryBlossomLeaves");
    pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\block\\cherryblossomleaves.png");
    pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
    pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
    pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
    pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
    return pModel;
}





}