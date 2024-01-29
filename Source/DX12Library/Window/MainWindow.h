#pragma once

#include "Common.h"

#include "Window/BaseWindow.h"

namespace DX12Library
{
    class MainWindow : public BaseWindow<MainWindow>
    {
    public:
        MainWindow(void);
        MainWindow(const MainWindow& other) = delete;
        MainWindow(MainWindow&& other) = delete;
        MainWindow& operator=(const MainWindow& other) = delete;
        MainWindow& operator=(MainWindow&& other) = delete;
        virtual ~MainWindow() = default;

        HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) override;
        PCWSTR GetWindowClassName(void) const override;
        LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) override;

        const UINT GetGameMode(void) const;
        const DirectionsInput& GetDirections(void) const;
        const MouseRelativeMovement& GetMouseRelativeMovement(void) const;
        void ResetMouseMovement(void);

    private:
        UINT m_gameMode;
        DirectionsInput m_directions;
        MouseRelativeMovement m_mouseRelativeMovement;
    };
}