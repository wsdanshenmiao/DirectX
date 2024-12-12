#include "../Common/D3D12.h"

using namespace DSM;
using namespace DirectX;
class Test : public D3D12App
{
public:
	Test(HINSTANCE hAppInst, const std::wstring& mainWndCaption, int clientWidth = 512, int clientHeight = 512);

private:


	// 通过 D3D12App 继承
	void OnUpdate(const CpuTimer& timer) override
	{
	}

	void OnRender(const CpuTimer& timer) override
	{
	}

};



int WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		Test app(hInstance, L"Shapes", 1024, 720);
		app.OnInit();
		return app.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

