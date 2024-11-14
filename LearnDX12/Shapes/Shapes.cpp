#include "Shapes.h"
#include "ImGuiManager.h"
#include "../Common/ConstantData.h"
#include "../Common/Vertex.h"
#include "../Common/Mesh.h"

using namespace DirectX;
using namespace DSM::Geometry;

namespace DSM {
	Shapes::Shapes(HINSTANCE hAppInst, const std::wstring& mainWndCaption, int clientWidth, int clientHeight)
		:D3D12App(hAppInst, mainWndCaption, clientWidth, clientHeight) {
	}

	Shapes::~Shapes()
	{
		if (m_D3D12Fence) {
			FlushCommandQueue();
		}
		ImGuiManager::ShutDown();
	}

	bool Shapes::OnInit()
	{
		if (!D3D12App::OnInit())
			return false;
		ImGuiManager::Create();
		if (!ImGuiManager::GetInstance().InitImGui(
			m_D3D12Device.Get(),
			m_hMainWnd,
			SwapChainBufferCount,
			m_BackBufferFormat))
			return false;

		// 为初始化资源重置命令列表
		ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));
		if (!InitResource())
			return false;
		// 提交命令列表
		ThrowIfFailed(m_CommandList->Close());
		ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		FlushCommandQueue();

		return true;
	}

	void Shapes::OnResize()
	{
		D3D12App::OnResize();
	}

	void Shapes::OnUpdate(const CpuTimer& timer)
	{
		auto& imgui = ImGuiManager::GetInstance();
		imgui.Update(timer);

		m_CurrFRIndex = (m_CurrFRIndex + 1) % FrameCount;
		m_CurrFrameResource = m_FrameResources[m_CurrFRIndex].get();

		// 若CPU过快，可能会超前一个循环，此时需要CPU等待
		if (m_CurrFrameResource->m_Fence != 0 &&
			m_D3D12Fence->GetCompletedValue() < m_CurrFrameResource->m_Fence) {
			WaitForGpu();
		}

		UpdateFrameResource(timer);
	}

	void Shapes::UpdateFrameResource(const CpuTimer& timer)
	{
		auto& imgui = ImGuiManager::GetInstance();

		XMMATRIX view = XMMatrixLookAtLH(
			XMVectorSet(m_EyePos.x, m_EyePos.y, m_EyePos.z, 1.0f),
			XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
			XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		auto world = XMMatrixScalingFromVector(XMVectorReplicate(imgui.m_Scale)) *
			XMMatrixRotationX(imgui.m_Phi) * XMMatrixRotationY(imgui.m_Theta) *
			XMMatrixTranslation(imgui.m_Dx, imgui.m_Dy, 0.0f);
		auto proj = XMMatrixPerspectiveFovLH(imgui.m_Fov, GetAspectRatio(), 1.0f, 1000.0f);

		auto detView = XMMatrixDeterminant(view);
		auto detProj = XMMatrixDeterminant(proj);
		auto detWorld = XMMatrixDeterminant(world);
		XMMATRIX viewProj = XMMatrixMultiply(view, proj);
		XMMATRIX invView = XMMatrixInverse(&detView, view);
		XMMATRIX invProj = XMMatrixInverse(&detProj, proj);
		XMMATRIX invWorld = XMMatrixInverse(&detWorld, world);

		PassConstants passConstants;
		XMStoreFloat4x4(&passConstants.m_View, XMMatrixTranspose(view));
		XMStoreFloat4x4(&passConstants.m_InvView, XMMatrixTranspose(invView));
		XMStoreFloat4x4(&passConstants.m_Proj, XMMatrixTranspose(proj));
		XMStoreFloat4x4(&passConstants.m_InvProj, XMMatrixTranspose(invProj));
		passConstants.m_EyePosW = m_EyePos;
		passConstants.m_RenderTargetSize = XMFLOAT2((float)m_ClientWidth, (float)m_ClientHeight);
		passConstants.m_InvRenderTargetSize = XMFLOAT2(1.0f / m_ClientWidth, 1.0f / m_ClientHeight);
		passConstants.m_NearZ = 1.0f;
		passConstants.m_FarZ = 1000.0f;
		passConstants.m_TotalTime = timer.TotalTime();
		passConstants.m_DeltaTime = timer.DeltaTime();

		auto& currPassCB = m_CurrFrameResource->m_ConstantBuffers.find("PassConstants")->second;
		currPassCB->CopyData(0, &passConstants, sizeof(PassConstants));

		ObjectConstants objectConstants;
		XMStoreFloat4x4(&objectConstants.m_World, XMMatrixTranspose(world));
		XMStoreFloat4x4(&objectConstants.m_WorldInvTranspos, invWorld);
		auto& currObjCB = m_CurrFrameResource->m_ConstantBuffers.find("ObjectConstants")->second;
		for (auto i = 0; i < m_Geometrys.size(); ++i) {
			currObjCB->CopyData(i, &objectConstants, sizeof(ObjectConstants));
		}
	}

	void Shapes::OnRender(const CpuTimer& timer)
	{
		auto cmdListAlloc = m_CurrFrameResource->m_CmdListAlloc;
		ThrowIfFailed(cmdListAlloc->Reset());
		ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), m_PSO.Get()));

		m_CommandList->RSSetViewports(1, &m_ScreenViewport);
		m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		if (m_Enable4xMsaa) {
			auto resolveToRt = CD3DX12_RESOURCE_BARRIER::Transition(
				m_MsaaRenderTarget.Get(),
				D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_CommandList->ResourceBarrier(1, &resolveToRt);

			auto msaaRtv = m_MsaaRtvHeap->GetCPUDescriptorHandleForHeapStart();
			auto msaaDsv = m_MsaaDsvHeap->GetCPUDescriptorHandleForHeapStart();
			m_CommandList->ClearRenderTargetView(msaaRtv, Colors::Pink, 0, nullptr);
			m_CommandList->ClearDepthStencilView(
				msaaDsv,
				D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
				1.f, 0, 0, nullptr);

			m_CommandList->OMSetRenderTargets(1, &msaaRtv, true, &msaaDsv);

			OnRenderScene(timer);

			// 转换资源状态
			D3D12_RESOURCE_BARRIER barriers[2] = {
				CD3DX12_RESOURCE_BARRIER::Transition(
					m_MsaaRenderTarget.Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
				CD3DX12_RESOURCE_BARRIER::Transition(
					GetCurrentBackBuffer(),
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_RESOLVE_DEST)
			};
			m_CommandList->ResourceBarrier(2, barriers);

			// 将Rt解析到后台缓冲区
			m_CommandList->ResolveSubresource(
				GetCurrentBackBuffer(), 0,
				m_MsaaRenderTarget.Get(), 0,
				m_BackBufferFormat);

			// 转换后台缓冲区
			auto destToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
				GetCurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RESOLVE_DEST,
				D3D12_RESOURCE_STATE_PRESENT);
			m_CommandList->ResourceBarrier(1, &destToPresent);
		}
		else {
			auto presentToRt = CD3DX12_RESOURCE_BARRIER::Transition(
				GetCurrentBackBuffer(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_CommandList->ResourceBarrier(1, &presentToRt);

			auto currentBbv = GetCurrentBackBufferView();
			auto dsv = GetDepthStencilView();
			m_CommandList->ClearRenderTargetView(currentBbv, Colors::Pink, 0, nullptr);
			m_CommandList->ClearDepthStencilView(
				dsv,
				D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
				1.f, 0, 0, nullptr);

			m_CommandList->OMSetRenderTargets(1, &currentBbv, true, &dsv);

			OnRenderScene(timer);

			auto RtToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
				GetCurrentBackBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT);
			m_CommandList->ResourceBarrier(1, &RtToPresent);
		}

		ThrowIfFailed(m_CommandList->Close());

		ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

		ThrowIfFailed(m_DxgiSwapChain->Present(0, 0));
		m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

		// 推进当前帧资源的围栏值
		m_CurrFrameResource->m_Fence = ++m_CurrentFence;
		m_CommandQueue->Signal(m_D3D12Fence.Get(), m_CurrentFence);
	}

	void Shapes::OnRenderScene(const CpuTimer& timer)
	{
		// 设置根签名
		ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
		m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
		// 设置当前帧资源的根描述符表
		int passCbvIndex = m_PassCbvOffset + m_CurrFRIndex;
		auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
		passCbvHandle.Offset(passCbvIndex, m_CbvSrvUavDescriptorSize);
		m_CommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);


		for (auto& geometry : m_Geometrys) {
			auto& g = geometry.second;
			auto vertexBV = g.GetVertexBufferView();
			auto indexBV = g.GetIndexBufferView();
			m_CommandList->IASetVertexBuffers(0, 1, &vertexBV);
			m_CommandList->IASetIndexBuffer(&indexBV);

			UINT objCbvIndex = m_CurrFRIndex * m_Geometrys.size();
			auto objCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
			objCbvHandle.Offset(objCbvIndex, m_CbvSrvUavDescriptorSize);
			m_CommandList->SetGraphicsRootDescriptorTable(0, objCbvHandle);

			auto& boxSubMesh = g.m_DrawArgs[geometry.first];
			m_CommandList->DrawIndexedInstanced(
				boxSubMesh.m_IndexCount,
				1,
				boxSubMesh.m_BaseVertexLocation,
				boxSubMesh.m_StarIndexLocation,
				0);
		}

		ImGuiManager::GetInstance().RenderImGui(m_CommandList.Get());
	}

	bool Shapes::InitResource()
	{
		m_EyePos = { 0.0f, 0.0f, -5.0f };

		m_VSByteCode = D3DUtil::CompileShader(L"Shader\\Color.hlsl", nullptr, "VS", "vs_5_0");
		m_PSByteCode = D3DUtil::CompileShader(L"Shader\\Color.hlsl", nullptr, "PS", "ps_5_0");

		CreateBox();
		InitFrameResourceCB();
		CreateCBV();
		CreateRootSignature();
		CreatePSO();

		return true;
	}

	bool Shapes::InitFrameResourceCB()
	{
		auto objCount = (UINT)m_Geometrys.size();
		m_PassCbvOffset = objCount * FrameCount;	// 帧常量的偏移量

		// 创建常量缓冲区描述符堆
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc{};
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NumDescriptors = m_PassCbvOffset + FrameCount;		// 前面放置物体常量的描述符，后面放置帧常量
		cbvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_D3D12Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_CbvHeap.GetAddressOf())));

		for (auto i = 0; i < FrameCount; ++i) {
			m_FrameResources[i] = std::make_unique<FrameResource>(m_D3D12Device.Get());
		}

		for (auto& resource : m_FrameResources) {
			resource->AddConstantBuffer(m_D3D12Device.Get(), sizeof(ObjectConstants), 1, "ObjectConstants");
			resource->AddConstantBuffer(m_D3D12Device.Get(), sizeof(PassConstants), 1, "PassConstants");
		}

		return true;
	}

	bool Shapes::CreateCBV()
	{
		auto objByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		auto passByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

		// 创建每个帧资源的常量缓冲区视图
		for (auto frameIndex = 0; frameIndex < FrameCount; ++frameIndex) {
			auto& currFrameResource = m_FrameResources[frameIndex];

			// 创建物体的常量缓冲区视图
			auto objCount = (UINT)m_Geometrys.size();
			auto objCB = currFrameResource->m_ConstantBuffers.find("ObjectConstants")->second->GetResource();
			for (std::size_t i = 0; i < objCount; ++i) {
				auto objCbvAdress = objCB->GetGPUVirtualAddress();	// 常量缓冲区的GPU虚拟首地址
				objCbvAdress += i * objByteSize;	// 对首地址进行便宜

				UINT objHeapIndex = frameIndex * objCount + i;
				// 获取描述符堆的首地址
				auto objHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
				objHandle.Offset(objHeapIndex, m_CbvSrvUavDescriptorSize);

				// 创建常量缓冲区视图
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
				cbvDesc.SizeInBytes = objByteSize;
				cbvDesc.BufferLocation = objCbvAdress;
				m_D3D12Device->CreateConstantBufferView(&cbvDesc, objHandle);
			}


			// 创建每一帧的常量缓冲区视图
			auto passCB = currFrameResource->m_ConstantBuffers.find("PassConstants")->second->GetResource();
			auto passCbvAdress = passCB->GetGPUVirtualAddress();

			int passHeapIndex = m_PassCbvOffset + frameIndex;	// 偏移到帧常量区
			auto passHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
			passHandle.Offset(passHeapIndex, m_CbvSrvUavDescriptorSize);

			// 创建常量缓冲区视图
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
			cbvDesc.SizeInBytes = passByteSize;
			cbvDesc.BufferLocation = passCbvAdress;
			m_D3D12Device->CreateConstantBufferView(&cbvDesc, passHandle);
		}

		return true;
	}

	bool Shapes::CreateBox()
	{
		// 设置顶点
		std::array<VertexPosColor, 8> vertexs = {
			VertexPosColor({ XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT4(Colors::White) }),
			VertexPosColor({ XMFLOAT3(-1.0f, +1.0f, -1.0f),XMFLOAT4(Colors::Black) }),
			VertexPosColor({ XMFLOAT3(+1.0f, +1.0f, -1.0f),XMFLOAT4(Colors::Red) }),
			VertexPosColor({ XMFLOAT3(+1.0f, -1.0f, -1.0f),XMFLOAT4(Colors::Green) }),
			VertexPosColor({ XMFLOAT3(-1.0f, -1.0f, +1.0f),XMFLOAT4(Colors::Blue) }),
			VertexPosColor({ XMFLOAT3(-1.0f, +1.0f, +1.0f),XMFLOAT4(Colors::Yellow) }),
			VertexPosColor({ XMFLOAT3(+1.0f, +1.0f, +1.0f),XMFLOAT4(Colors::Cyan) }),
			VertexPosColor({ XMFLOAT3(+1.0f, -1.0f, +1.0f),XMFLOAT4(Colors::Magenta) })
		};

		// 设置索引
		std::array<std::uint16_t, 36> indices =
		{
			// front face
			0, 1, 2,
			0, 2, 3,
			// back face
			4, 6, 5,
			4, 7, 6,
			// left face
			4, 5, 1,
			4, 1, 0,
			// right face
			3, 2, 6,
			3, 6, 7,
			// top face
			1, 5, 6,
			1, 6, 2,
			// bottom face
			4, 0, 3,
			4, 3, 7
		};


		const UINT64 vbByteSize = vertexs.size() * sizeof(VertexPosColor);
		const UINT64 ibByteSize = indices.size() * sizeof(std::uint16_t);

		m_Geometrys.insert(std::make_pair("Box", MeshData{}));
		auto& box = m_Geometrys["Box"];

		ThrowIfFailed(D3DCreateBlob(vbByteSize, box.m_VertexBufferCPU.GetAddressOf()));
		memcpy(box.m_VertexBufferCPU->GetBufferPointer(), vertexs.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, box.m_IndexBufferCPU.GetAddressOf()));
		memcpy(box.m_IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		// 创建顶点和索引缓冲区
		box.m_VertexBufferGPU = D3DUtil::CreateDefaultBuffer(
			m_D3D12Device.Get(), m_CommandList.Get(), vertexs.data(), vbByteSize, box.m_VertexBufferUploader);
		box.m_IndexBufferGPU = D3DUtil::CreateDefaultBuffer(
			m_D3D12Device.Get(), m_CommandList.Get(), indices.data(), ibByteSize, box.m_IndexBufferUploader);

		box.m_VertexBufferByteSize = vbByteSize;
		box.m_IndexBufferByteSize = ibByteSize;
		box.m_VertexByteStride = sizeof(VertexPosColor);
		box.m_IndexFormat = DXGI_FORMAT_R16_UINT;

		SubmeshData subMesh{};
		subMesh.m_IndexCount = (UINT)indices.size();
		subMesh.m_StarIndexLocation = 0;
		subMesh.m_BaseVertexLocation = 0;

		box.m_DrawArgs.emplace(std::make_pair("Box", subMesh));

		return true;
	}

	bool Shapes::CreateRootSignature()
	{
		auto CBSize = (UINT)m_FrameResources[0]->m_ConstantBuffers.size();
		// 创建根签名
		std::vector<CD3DX12_DESCRIPTOR_RANGE> cbvTables(CBSize);
		std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameters(CBSize);
		for (UINT i = 0; i < CBSize; ++i) {
			// 创建描述符表
			cbvTables[i].Init(
				D3D12_DESCRIPTOR_RANGE_TYPE_CBV,	// 描述符表的类型
				1,		// 表中的描述符数量
				i);		// 将描述符区域绑定到基准着色器寄存器
			slotRootParameters[i].InitAsDescriptorTable(
				1,				// 描述符区域的数量
				&cbvTables[i]);		// 指向描述符数组的指针
		}

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
			2, slotRootParameters.data(), 0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// 创建含一个槽位的根签名
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		// 创建根签名之前需要对根签名的描述布局进行序列化，转换为以ID3DBlob接口表示的序列化数据格式
		auto hr = D3D12SerializeRootSignature(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf());

		if (errorBlob) {
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		// 创建根签名
		ThrowIfFailed(m_D3D12Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(m_RootSignature.GetAddressOf())));

		return true;
	}

	bool Shapes::CreatePSO()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.VS = {
			m_VSByteCode->GetBufferPointer(),
			m_VSByteCode->GetBufferSize()
		};
		psoDesc.PS = {
			m_PSByteCode->GetBufferPointer(),
			m_PSByteCode->GetBufferSize()
		};
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.InputLayout = {
			VertexPosColor::GetInputLayOut().data(),
			(UINT)VertexPosColor::GetInputLayOut().size()
		};
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = m_BackBufferFormat;
		psoDesc.SampleDesc.Count = m_Enable4xMsaa ? 4 : 1;
		psoDesc.SampleDesc.Quality = m_Enable4xMsaa ? (m_4xMsaaQuality - 1) : 0;
		psoDesc.DSVFormat = m_DepthStencilFormat;
		m_D3D12Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(m_PSO.GetAddressOf()));

		return true;
	}

	void Shapes::WaitForGpu()
	{
		HANDLE eventHandle = CreateEvent(nullptr, false, false, nullptr);
		ThrowIfFailed(m_D3D12Fence->SetEventOnCompletion(m_CurrFrameResource->m_Fence, eventHandle));
		if (eventHandle != 0) {
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}


}
