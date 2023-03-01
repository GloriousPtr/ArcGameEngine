#include "arcpch.h"
#include "Dx12Window.h"

#include "Arc/Core/Input.h"
#include "Arc/Events/ApplicationEvent.h"
#include "Arc/Events/MouseEvent.h"
#include "Arc/Events/KeyEvent.h"

#include "Platform/OpenGL/OpenGLContext.h"

#include <comutil.h>
#include <imgui.h>

namespace ArcEngine
{
	// Forward declare message handler from imgui_impl_win32.cpp
	//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Win32 message handler
	// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
	// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
	// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
	// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		//if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			//return true;

		switch (msg)
		{
			case WM_SYSCOMMAND:
			{
				if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
					return 0;
				break;
			}
		}

		// Handle and dispatch events
		auto windowData = reinterpret_cast<Dx12Window::WindowData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (windowData)
		{
			switch (msg)
			{
				case WM_SYSCOMMAND:
					if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
						return 0;
					break;

				case WM_MOUSEMOVE:
				{
					int xPos = LOWORD(lParam);
					int yPos = HIWORD(lParam);
					MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
					windowData->EventCallback(event);
					return 0;
				}

				case WM_MOUSEWHEEL:
				{
					int yOffset = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
					MouseScrolledEvent event(0.0f, static_cast<float>(yOffset));
					windowData->EventCallback(event);
					return 0;
				}

				// Mouse Button Release
				case WM_LBUTTONUP:
				{
					MouseButtonReleasedEvent event(Mouse::ButtonLeft);
					windowData->EventCallback(event);
					return 0;
				}
				case WM_RBUTTONUP:
				{
					MouseButtonReleasedEvent event(Mouse::ButtonRight);
					windowData->EventCallback(event);
					return 0;
				}
				case WM_MBUTTONUP:
				{
					MouseButtonReleasedEvent event(Mouse::ButtonMiddle);
					windowData->EventCallback(event);
					return 0;
				}

				// Mouse Button Pressed
				case WM_LBUTTONDOWN:
				{
					MouseButtonPressedEvent event(Mouse::ButtonLeft);
					windowData->EventCallback(event);
					return 0;
				}
				case WM_RBUTTONDOWN:
				{
					MouseButtonPressedEvent event(Mouse::ButtonRight);
					windowData->EventCallback(event);
					return 0;
				}
				case WM_MBUTTONDOWN:
				{
					MouseButtonPressedEvent event(Mouse::ButtonMiddle);
					windowData->EventCallback(event);
					return 0;
				}

				// Key typed
				case WM_CHAR:
				{
					auto key = static_cast<KeyCode>(wParam);
					KeyTypedEvent event(static_cast<KeyCode>(key));
					windowData->EventCallback(event);
					return 0;
				}

				// Key Pressed/Release
				case WM_KEYUP:
				{
					auto key = static_cast<KeyCode>(wParam);
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					windowData->EventCallback(event);
					return 0;
				}
				case WM_KEYDOWN:
				{
					auto key = static_cast<KeyCode>(wParam);
					auto repeatCount = static_cast<uint16_t>(lParam);
					KeyPressedEvent event(static_cast<KeyCode>(key), repeatCount);
					windowData->EventCallback(event);
					return 0;
				}

				// Resize
				case WM_SIZE:
				{
					int width = LOWORD(lParam);
					int height = HIWORD(lParam);
					windowData->Width = width;
					windowData->Height = height;
					WindowResizeEvent event(width, height);
					windowData->EventCallback(event);
					return 0;
				}

				// Close
				case WM_DESTROY:
				{
					WindowCloseEvent event;
					windowData->EventCallback(event);
					PostQuitMessage(0);
					return 0;
				}
			}
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	uint8_t Dx12Window::s_WindowCount = 0;

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
		++s_WindowCount;

		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();

		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&m_Data);
	}

	void Dx12Window::Shutdown() const
	{
		ARC_PROFILE_SCOPE()

		DestroyWindow(static_cast<HWND>(m_Window));
		const _bstr_t wideTitle(m_Data.Title.c_str());
		UnregisterClassW(wideTitle, static_cast<HINSTANCE>(m_Data.HInstance));
		--s_WindowCount;
	}
}
