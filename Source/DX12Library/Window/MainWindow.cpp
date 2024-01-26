#include "MainWindow.h"

namespace DX12Library
{
    MainWindow::MainWindow()
        : BaseWindow()
        , m_gameMode(1)
        , m_directions()
        , m_mouseRelativeMovement()
    {
    }

    HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName)
    {
        return initialize(hInstance, nCmdShow, pszWindowName, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
    }

    PCWSTR MainWindow::GetWindowClassName() const
    {
        return m_pszWindowName;
    }

    LRESULT MainWindow::HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;

        // Register input device for mouse raw input
        RAWINPUTDEVICE Rid[1];
        Rid[0].usUsagePage = static_cast<USHORT>(0x01);
        Rid[0].usUsage = static_cast<USHORT>(0x02);
        Rid[0].dwFlags = RIDEV_INPUTSINK;
        Rid[0].hwndTarget = m_hWnd;
        RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

        switch (uMsg)
        {
        case WM_INPUT:
        {
            // Determine mouse relative movement from raw input data
            UINT dwSize = sizeof(RAWINPUT);
            static BYTE lpb[sizeof(RAWINPUT)];

            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

            RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb);

            if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                m_mouseRelativeMovement.X = raw->data.mouse.lLastX;
                m_mouseRelativeMovement.Y = raw->data.mouse.lLastY;
            }
            break;
        }

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
            case 0x31:
            case VK_NUMPAD1:
                m_gameMode = 1;
                break;

            case 0x32:
            case VK_NUMPAD2:
                m_gameMode = 2;
                break;

            case 0x33:
            case VK_NUMPAD3:
                m_gameMode = 3;
                break;

            // Determine directions True
            case 0x57:      // W key - Forward
                m_directions.bFront = TRUE;
                break;

            case 0x41:      // A key - Left
                m_directions.bLeft = TRUE;
                break;

            case 0x53:      // S key - Back
                m_directions.bBack = TRUE;
                break;

            case 0x44:      // D key - Right
                m_directions.bRight = TRUE;
                break;

            case VK_SPACE:  // Space Bar - Up
                m_directions.bUp = TRUE;
                break;

            case VK_SHIFT:  // Shift key - Down
                m_directions.bDown = TRUE;
                break;

            case VK_ESCAPE:
				if (MessageBox(m_hWnd,
					L"Really quit?",
					PSZ_TITLE,
					MB_OKCANCEL) == IDOK)
				{
					DestroyWindow(m_hWnd);
				}
				// else: user canceled. Do nothing.
                break;

            default:
                break;
            }
            break;
        }

        case WM_KEYUP:
        {
            // Determine directions False
            switch (wParam)
            {
            case 0x57:      // W key - Forward
                m_directions.bFront = FALSE;
                break;

            case 0x41:      // A key - Left
                m_directions.bLeft = FALSE;
                break;

            case 0x53:      // S key - Back
                m_directions.bBack = FALSE;
                break;

            case 0x44:      // D key - Right
                m_directions.bRight = FALSE;
                break;

            case VK_SPACE:  // Space Bar - Up
                m_directions.bUp = FALSE;
                break;

            case VK_SHIFT:  // Shift key - Down
                m_directions.bDown = FALSE;
                break;

            default:
                break;
            }
            break;
        }

        case WM_PAINT:
        {
            hdc = BeginPaint(m_hWnd, &ps);
            EndPaint(m_hWnd, &ps);
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        case WM_CLOSE:
        {
            if (MessageBox(m_hWnd,
                L"Really quit?",
                PSZ_TITLE,
                MB_OKCANCEL) == IDOK)
            {
                DestroyWindow(m_hWnd);
            }
            // else: user canceled. Do nothing.
            return S_OK;
        }

        default:
            return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }

        return S_OK;
    }

    const UINT MainWindow::GetGameMode() const
    {
        return m_gameMode;
    }

    const DirectionsInput& MainWindow::GetDirections() const
    {
        return m_directions;
    }

    const MouseRelativeMovement& MainWindow::GetMouseRelativeMovement() const
    {
        return m_mouseRelativeMovement;
    }

    void MainWindow::ResetMouseMovement()
    {
        m_mouseRelativeMovement =
        {
            .X = 0l,
            .Y = 0l
        };
    }
}