#include "arcpch.h"
#include "Dx12Window.h"

#include "Platform/OpenGL/OpenGLContext.h"

#include <comutil.h>
#include <imgui.h>

namespace ArcEngine
{
	// Forward declare message handler from imgui_impl_win32.cpp
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Win32 message handler
	// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
	// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		}
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}

	Dx12Window::Dx12Window(const WindowProps& props)
	{
		ARC_PROFILE_SCOPE()

		Init(props);
	}

	Dx12Window::~Dx12Window()
	{
		ARC_PROFILE_SCOPE()

		Shutdown();
	}

	void Dx12Window::OnUpdate()
	{
		ARC_PROFILE_SCOPE()

		// Poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		m_Context->SwapBuffers();
	}

	void Dx12Window::Init(const WindowProps& props)
	{
		ARC_PROFILE_SCOPE()

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		ARC_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		// Create application window
		//ImGui_ImplWin32_EnableDpiAwareness();

		const _bstr_t wideTitle(props.Title.c_str());
		WNDCLASSEXW wc =
		{
			sizeof(wc),
			CS_CLASSDC,
			WndProc,
			0L,	0L,
			GetModuleHandle(nullptr),
			nullptr, nullptr, nullptr, nullptr,
			wideTitle,
			nullptr
		};

		RegisterClassExW(&wc);
		const HWND hwnd = CreateWindowW(wc.lpszClassName, wideTitle, WS_OVERLAPPEDWINDOW, 0, 0, props.Width, props.Height, nullptr, nullptr, wc.hInstance, nullptr);

		m_Window = hwnd;
		m_Data.HInstance = wc.hInstance;

		// Show the window
		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();
	}

	void Dx12Window::Shutdown() const
	{
		ARC_PROFILE_SCOPE()

		DestroyWindow(static_cast<HWND>(m_Window));
		const _bstr_t wideTitle(m_Data.Title.c_str());
		UnregisterClassW(wideTitle, static_cast<HINSTANCE>(m_Data.HInstance));
	}
}
