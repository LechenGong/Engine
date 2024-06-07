#pragma once
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include <string>

#include "Engine/Core/EngineCommon.hpp"

class InputSystem;

struct WindowConfig
{
	InputSystem* m_inputSystem = nullptr;
	std::string m_windowTitle = "Untitled App";
	float m_clientAspect = 2.0f;
	bool m_isFullScreen = false;
};

class Window
{
public:
	Window( WindowConfig const& config );
	~Window();

	void StartUp();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	WindowConfig const& GetConfig() const;
	float GetAspect() const;
	void* GetDeviceContext() const;

	void* GetHwnd() const;
	static bool IsWindowFocused();
	IntVec2 GetClientDimensions() const;

	static Window* GetMainWindowInstance();

protected:
	void CreateOSWindow();
	void RunMessagePump();

protected:
	WindowConfig m_config;

protected:
	static Window* s_theWindow;

	void* m_hwnd = nullptr;
	void* m_displayDeviceContext = nullptr;

	IntVec2 m_clientDimensions = IntVec2::ZERO;
};

LRESULT CALLBACK WindowsMessageHandlingProcedure( HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam );

bool InputSystemKeyDown( unsigned int param );
bool InputSystemKeyUp( unsigned int param );