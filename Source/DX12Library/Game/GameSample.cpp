#include "GameSample.h"
#include <dxgidebug.h>

namespace DX12Library
{
    GameSample::GameSample(_In_ PCWSTR pszGameName)
        : m_pszGameName(pszGameName)
        , m_mainWindow(std::make_unique<MainWindow>())
        , m_camera(XMVectorSet(0.0f, 5.0f, -20.0f, 0.0f))
    {
    }

    GameSample::~GameSample()
    {
    }

    HRESULT GameSample::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
    {
        ThrowIfFailed(m_mainWindow->Initialize(hInstance, nCmdShow, m_pszGameName));

        InitDevice();

        return S_OK;
    }

    INT GameSample::Run(void)
    {
        // Initialize time
        LARGE_INTEGER LastTime;
        LARGE_INTEGER CurrentTime;
        LARGE_INTEGER Frequency;

        FLOAT deltaTime;

        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&LastTime);

        // Main message loop
        MSG msg = { 0 };
        while (WM_QUIT != msg.message)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                // Call WndProc Function
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                // Update our time
                QueryPerformanceCounter(&CurrentTime);
                deltaTime = static_cast<FLOAT>(CurrentTime.QuadPart - LastTime.QuadPart);
                deltaTime /= static_cast<FLOAT>(Frequency.QuadPart);
                LastTime = CurrentTime;

                // Handle input
                HandleInput(m_mainWindow->GetDirections(), m_mainWindow->GetMouseRelativeMovement(), deltaTime);
                m_mainWindow->ResetMouseMovement();

                // Render
                Update(deltaTime);
                Render();
            }
        }

        // Destroy
        CleanupDevice();

#ifdef _DEBUG
        IDXGIDebug1* pdxgiDebug = nullptr;
        DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), reinterpret_cast<void**>(&pdxgiDebug));
        pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
        pdxgiDebug->Release();
#endif //_DEBUG

        return static_cast<INT>(msg.wParam);
    }

    PCWSTR GameSample::GetGameName(void) const
    {
        return m_pszGameName;
    }

    std::unique_ptr<MainWindow>& GameSample::GetWindow(void)
    {
        return m_mainWindow;
    }
}