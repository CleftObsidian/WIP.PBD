#pragma once

#include "Common.h"

#include "Window/BaseWindow.h"

namespace DX12Library
{
    class MainWindow : public BaseWindow<MainWindow>
    {
    public:
        MainWindow();
        MainWindow(const MainWindow& other) = delete;
        MainWindow(MainWindow&& other) = delete;
        MainWindow& operator=(const MainWindow& other) = delete;
        MainWindow& operator=(MainWindow&& other) = delete;
        virtual ~MainWindow() = default;

        HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) override;
        PCWSTR GetWindowClassName() const override;
        LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) override;

        const UINT GetGameMode() const;
        const DirectionsInput& GetDirections() const;
        const MouseRelativeMovement& GetMouseRelativeMovement() const;
        void ResetMouseMovement();

    private:
        UINT m_gameMode;
        DirectionsInput m_directions;
        MouseRelativeMovement m_mouseRelativeMovement;
    };
}