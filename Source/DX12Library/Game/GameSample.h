#pragma once

#include "DXSampleHelper.h"
#include "Window/MainWindow.h"
#include "Camera/Camera.h"

using namespace DirectX;

namespace DX12Library
{
	class GameSample
	{
	public:
		GameSample(_In_ PCWSTR pszGameName);
		virtual ~GameSample();

		HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow);
		INT Run(void);

		virtual void InitDevice(void) = 0;
		virtual void CleanupDevice(void) = 0;
		virtual void HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime) = 0;
		virtual void Update(_In_ FLOAT deltaTime) = 0;
		virtual void Render(void) = 0;

		PCWSTR GetGameName(void) const;
		std::unique_ptr<MainWindow>& GetWindow(void);

	protected:
		static const UINT FRAMECOUNT = 2;

		PCWSTR m_pszGameName;
		std::unique_ptr<MainWindow> m_mainWindow;

		// Pipeline objects.
		CD3DX12_VIEWPORT m_viewport;
		CD3DX12_RECT m_scissorRect;

		Camera m_camera;
	};
}