#include "Light.h"
#include "ImguiManager.h"
#include "../Common/Vertex.h"

using namespace DirectX;

namespace DSM {
	Light::Light(HINSTANCE hAppInst, const std::wstring& mainWndCaption, int clientWidth, int clientHeight)
		:D3D12App(hAppInst, mainWndCaption, clientWidth, clientHeight) {
	}

	bool Light::OnInit()
	{
		if (!D3D12App::OnInit()) {
			return false;
		}

		ImguiManager::Create();
		if (!ImguiManager::GetInstance().InitImGui(
			m_D3D12Device.Get(),
			m_hMainWnd,
			FrameCount,
			m_BackBufferFormat)) {
			return false;
		}

		// 初始化相关资源
		ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));
		if (!InitResource()) {
			return false;
		}
		// 提交列表
		ThrowIfFailed(m_CommandList->Close());
		ID3D12CommandList* cmdLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(1, cmdLists);
		FlushCommandQueue();

		return true;
	}

	void Light::OnUpdate(const CpuTimer& timer)
	{
		ImguiManager::GetInstance().Update(timer);

		m_CurrFRIndex = (m_CurrFRIndex + 1) % FrameCount;
		m_CurrFrameResource = m_FrameResources[m_CurrFRIndex].get();

		// 若CPU过快，可能会超前一个循环，此时需要CPU等待
		if (m_CurrFrameResource->m_Fence != 0 &&
			m_D3D12Fence->GetCompletedValue() < m_CurrFrameResource->m_Fence) {
			WaitForGPU();
		}


	}

	void Light::OnRender(const CpuTimer& timer)
	{
		auto& cmdListAlloc = m_CurrFrameResource->m_CmdListAlloc;
		ThrowIfFailed(cmdListAlloc->Reset());

		if (ImguiManager::GetInstance().m_EnableWireFrame) {
			auto& pso = m_Enable4xMsaa ? m_PSOs["WireFrameMSAA"] : m_PSOs["WireFrame"];
			ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), pso.Get()));
		}
		else {
			auto& pso = m_Enable4xMsaa ? m_PSOs["OpaqueMSAA"] : m_PSOs["Opaque"];
			ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), pso.Get()));
		}

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

			OnRenderScene();

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

			OnRenderScene();

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

	void Light::OnRenderScene()
	{
		// 设置根签名
		m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
		// 设置当前帧资源的根描述符表
		ID3D12DescriptorHeap* descriptorHeaps[] = { m_PassCbv.Get() };
		m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		auto passCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_PassCbv->GetGPUDescriptorHandleForHeapStart());
		passCbvHandle.Offset(m_CurrFRIndex, m_CbvSrvUavDescriptorSize);
		m_CommandList->SetGraphicsRootDescriptorTable(1, passCbvHandle);

		UINT currMeshIndex = 0;
		for (const auto& [name, obj] : m_Objects) {
			for (const auto& item : obj.GetAllRenderItems()) {
				auto verticesBV = item->m_Mesh->GetVertexBufferView();
				auto indicesBV = item->m_Mesh->GetIndexBufferView();
				m_CommandList->IASetVertexBuffers(0, 1, &verticesBV);
				m_CommandList->IASetIndexBuffer(&indicesBV);

				ID3D12DescriptorHeap* objDescriptorHeaps[] = { m_ObjCbv.Get() };
				m_CommandList->SetDescriptorHeaps(_countof(objDescriptorHeaps), objDescriptorHeaps);
				auto objCbvIndex = m_CurrFRIndex * m_RenderObjCount + item->m_RenderCBIndex;
				auto objCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_ObjCbv->GetGPUDescriptorHandleForHeapStart());
				objCbvHandle.Offset(objCbvIndex, m_CbvSrvUavDescriptorSize);
				m_CommandList->SetGraphicsRootDescriptorTable(0, objCbvHandle);

				auto& submesh = item->m_Mesh->m_DrawArgs[item->m_Name];
				m_CommandList->DrawIndexedInstanced(
					submesh.m_IndexCount,
					1,
					submesh.m_StarIndexLocation,
					submesh.m_BaseVertexLocation,
					0);
			}
		}
		ImguiManager::GetInstance().RenderImGui(m_CommandList.Get());
	}

	void Light::WaitForGPU()
	{
		HANDLE eventHandle = CreateEvent(nullptr, false, false, nullptr);
		ThrowIfFailed(m_D3D12Fence->SetEventOnCompletion(m_CurrFrameResource->m_Fence, eventHandle));
		if (eventHandle != 0) {
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	}

	bool Light::ImportModel()
	{
	}

	bool Light::InitResource()
	{
		ImportModel();
		CreateShaderBlob();
		CreateFrameResource();
		CreateCBV();
		CreateRootSignature();
		CreatePSO();

		return true;
	}

	void Light::CreateShaderBlob()
	{
		auto colorVS = D3DUtil::CompileShader(L"Shader\\Color.hlsl", nullptr, "VS", "vs_5_0");
		auto colorPS = D3DUtil::CompileShader(L"Shader\\Color.hlsl", nullptr, "PS", "ps_5_0");
		m_VSByteCodes.insert(std::make_pair("Color", colorVS));
		m_PSByteCodes.insert(std::make_pair("Color", colorPS));

	}

	/// <summary>
	/// 创建帧资源
	/// </summary>
	void Light::CreateFrameResource()
	{
		UINT renderItemSize = 0;
		for (const auto& [name, obj] : m_Objects) {
			renderItemSize += (UINT)obj.GetRenderItemsCount();
		}
		m_RenderObjCount = std::max<UINT>(renderItemSize, 1);

		for (auto& frameResource : m_FrameResources) {
			frameResource = std::make_unique<FrameResource>(m_D3D12Device.Get());
			// 添加常量缓冲区
			frameResource->AddConstantBuffer(
				m_D3D12Device.Get(),
				sizeof(ObjectConstants),
				m_RenderObjCount,
				"ObjectConstants");
			frameResource->AddConstantBuffer(
				m_D3D12Device.Get(),
				sizeof(PassConstants),
				FrameCount,
				"PassConstants");
		}
	}

	void Light::CreateCBV()
	{
		// 创建常量描述符堆
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.NumDescriptors = m_RenderObjCount * FrameCount;
		heapDesc.NodeMask = 0;
		ThrowIfFailed(m_D3D12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_ObjCbv.GetAddressOf())));
		heapDesc.NumDescriptors = FrameCount;
		ThrowIfFailed(m_D3D12Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_PassCbv.GetAddressOf())));

		// 计算常量缓冲区的大小
		auto objCbvByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
		auto passCbvByteSize = D3DUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

		// 创建对应视图 
		for (UINT frameIndex = 0; frameIndex < FrameCount; ++frameIndex) {
			auto& constBuffer = m_FrameResources[frameIndex]->m_ConstantBuffers;
			auto passCB = constBuffer.find("PassConstants")->second->GetResource();
			for (UINT i = 0; i < m_RenderObjCount; ++i) {
				auto objCB = constBuffer.find("ObjectConstants")->second->GetResource();
				// 常量缓冲区虚拟地址
				auto objCbvAdress = objCB->GetGPUVirtualAddress();
				objCbvAdress += i * objCbvByteSize;

				D3D12_CONSTANT_BUFFER_VIEW_DESC objCbvDesc{};
				objCbvDesc.BufferLocation = objCbvAdress;
				objCbvDesc.SizeInBytes = objCbvByteSize;

				auto handleOffset = frameIndex * m_RenderObjCount + i;
				auto objHandle = m_ObjCbv->GetCPUDescriptorHandleForHeapStart();
				objHandle.ptr += handleOffset * m_CbvSrvUavDescriptorSize;

				m_D3D12Device->CreateConstantBufferView(&objCbvDesc, objHandle);
			}

			auto passCbvAdress = passCB->GetGPUVirtualAddress();
			passCbvAdress += frameIndex * passCbvByteSize;

			D3D12_CONSTANT_BUFFER_VIEW_DESC passCbvDesc{};
			passCbvDesc.BufferLocation = passCbvAdress;
			passCbvDesc.SizeInBytes = passCbvByteSize;

			auto passHandle = m_PassCbv->GetCPUDescriptorHandleForHeapStart();
			passHandle.ptr += frameIndex * m_CbvSrvUavDescriptorSize;

			m_D3D12Device->CreateConstantBufferView(&passCbvDesc, passHandle);
		}
	}

	void Light::CreateRootSignature()
	{
		auto constBufferSize = m_FrameResources[0]->m_ConstantBuffers.size();
		std::vector<CD3DX12_DESCRIPTOR_RANGE> cbvTables(constBufferSize);
		std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameter(constBufferSize);
		for (UINT i = 0; i < constBufferSize; ++i) {
			cbvTables[i].Init(
				D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
				1, i);
			slotRootParameter[i].InitAsDescriptorTable(
				1, &cbvTables[i]);
		}

		D3D12_ROOT_SIGNATURE_DESC rootSignatrueDesc{};
		rootSignatrueDesc.NumParameters = (UINT)constBufferSize;
		rootSignatrueDesc.pParameters = slotRootParameter.data();
		rootSignatrueDesc.NumStaticSamplers = 0;
		rootSignatrueDesc.pStaticSamplers = nullptr;
		rootSignatrueDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ComPtr<ID3DBlob> errorBlob;
		ComPtr<ID3DBlob> serilizedRootsig;
		auto hr = D3D12SerializeRootSignature(
			&rootSignatrueDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serilizedRootsig.GetAddressOf(),
			errorBlob.GetAddressOf());

		if (errorBlob) {
			::OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(m_D3D12Device->CreateRootSignature(
			0,
			serilizedRootsig->GetBufferPointer(),
			serilizedRootsig->GetBufferSize(),
			IID_PPV_ARGS(m_RootSignature.GetAddressOf())));
	}

	void Light::CreatePSO()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		psoDesc.pRootSignature = m_RootSignature.Get();
		psoDesc.VS = {
			m_VSByteCodes["Color"]->GetBufferPointer(),
			m_VSByteCodes["Color"]->GetBufferSize()
		};
		psoDesc.PS = {
			m_PSByteCodes["Color"]->GetBufferPointer(),
			m_PSByteCodes["Color"]->GetBufferSize()
		};
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.InputLayout = {
			VertexPosLColor::GetInputLayout().data(),
			(UINT)VertexPosLColor::GetInputLayout().size()
		};
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = m_BackBufferFormat;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.DSVFormat = m_DepthStencilFormat;
		ThrowIfFailed(m_D3D12Device->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(m_PSOs["Opaque"].GetAddressOf())));

		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		ThrowIfFailed(m_D3D12Device->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(m_PSOs["WireFrame"].GetAddressOf())));

		psoDesc.SampleDesc.Count = 4;
		psoDesc.SampleDesc.Quality = (m_4xMsaaQuality - 1);
		ThrowIfFailed(m_D3D12Device->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(m_PSOs["WireFrameMSAA"].GetAddressOf())));

		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		ThrowIfFailed(m_D3D12Device->CreateGraphicsPipelineState(
			&psoDesc, IID_PPV_ARGS(m_PSOs["OpaqueMSAA"].GetAddressOf())));
	}
}
