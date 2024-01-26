#pragma once

#include "Common.h"

namespace DX12Library
{
	template <class DerivedType>
	class BaseWindow
	{
	public:
		static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

        BaseWindow();
        BaseWindow(const BaseWindow& rhs) = delete;
        BaseWindow(BaseWindow&& rhs) = delete;
        BaseWindow& operator=(const BaseWindow& rhs) = delete;
        BaseWindow& operator=(BaseWindow&& rhs) = delete;
        virtual ~BaseWindow() = default;

        virtual HRESULT Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName) = 0;
        virtual PCWSTR GetWindowClassName() const = 0;
        virtual LRESULT HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) = 0;

        HWND GetWindow() const;

    protected:
        HRESULT initialize(
            _In_ HINSTANCE hInstance,
            _In_ INT nCmdShow,
            _In_ PCWSTR pszWindowName,
            _In_ DWORD dwStyle,
            _In_opt_ INT x = CW_USEDEFAULT,
            _In_opt_ INT y = CW_USEDEFAULT,
            _In_opt_ INT nWidth = CW_USEDEFAULT,
            _In_opt_ INT nHeight = CW_USEDEFAULT,
            _In_opt_ HWND hWndParent = nullptr,
            _In_opt_ HMENU hMenu = nullptr
        );

        HINSTANCE m_hInstance;
        HWND m_hWnd;
        LPCWSTR m_pszWindowName;
	};

    template <class DerivedType>
    LRESULT CALLBACK BaseWindow<DerivedType>::WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        DerivedType* pThis = nullptr;

        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<DerivedType*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
            pThis->m_hWnd = hWnd;
        }
        else
        {
            pThis = reinterpret_cast<DerivedType*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    template <class DerivedType>
    BaseWindow<DerivedType>::BaseWindow()
        : m_hInstance(nullptr)
        , m_hWnd(nullptr)
        , m_pszWindowName(nullptr)
    {
    }

    template <class DerivedType>
    HWND BaseWindow<DerivedType>::GetWindow() const
    {
        return m_hWnd;
    }

    template <class DerivedType>
    HRESULT BaseWindow<DerivedType>::initialize(
        _In_ HINSTANCE hInstance,
        _In_ INT nCmdShow,
        _In_ PCWSTR pszWindowName,
        _In_ DWORD dwStyle,
        _In_opt_ INT x,
        _In_opt_ INT y,
        _In_opt_ INT nWidth,
        _In_opt_ INT nHeight,
        _In_opt_ HWND hWndParent,
        _In_opt_ HMENU hMenu
    )
    {
        // Initialize member variables
        m_hInstance = hInstance;
        m_pszWindowName = pszWindowName;

        // Register class
        WNDCLASSEX wcex =
        {
            .cbSize = static_cast<UINT>(sizeof(WNDCLASSEX)),
            .style = static_cast<UINT>(CS_HREDRAW | CS_VREDRAW),
            .lpfnWndProc = WindowProc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = m_hInstance,
            .hIcon = LoadIcon(m_hInstance, reinterpret_cast<LPCTSTR>(IDC_MYICON)),
            .hCursor = LoadCursor(nullptr, IDC_ARROW),
            .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
            .lpszMenuName = nullptr,
            .lpszClassName = GetWindowClassName(),
            .hIconSm = LoadIcon(m_hInstance, reinterpret_cast<LPCTSTR>(IDC_MYICON))
        };

        if (!RegisterClassEx(&wcex))
        {
            DWORD dwError = GetLastError();

            MessageBox(
                nullptr,
                L"Call to RegisterClassEx failed!",
                PSZ_TITLE,
                NULL
            );

            if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            {
                return HRESULT_FROM_WIN32(dwError);
            }

            return E_FAIL;
        }

        // Create window
        RECT rc = { 0, 0, nWidth, nHeight };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        m_hWnd = CreateWindowEx(
            0,
            GetWindowClassName(),
            m_pszWindowName,
            dwStyle,
            x,
            y,
            nWidth,
            nHeight,
            hWndParent,
            hMenu,
            m_hInstance,
            this
        );

        if (!m_hWnd)
        {
            DWORD dwError = GetLastError();

            MessageBox(
                nullptr,
                L"Call to CreateWindowEx failed!",
                PSZ_TITLE,
                NULL
            );

            if (dwError != ERROR_CLASS_ALREADY_EXISTS)
            {
                return HRESULT_FROM_WIN32(dwError);
            }

            return E_FAIL;
        }

        ShowWindow(m_hWnd, nCmdShow);

        return S_OK;
    }
}