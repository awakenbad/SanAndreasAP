#include "UIManager.h"

#include "FastTravel.h"
#include <filesystem>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace 
{
	WNDPROC originalWndProc = nullptr;
	bool overlayVisible = false;

	bool isInputMessage(UINT msg)
	{
		return (msg >= WM_KEYFIRST && msg <= WM_KEYLAST) ||
			(msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST);
	}

	LRESULT CALLBACK HookedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
			return true;
		if (overlayVisible && isInputMessage(msg))
			return 0;
		return CallWindowProc(originalWndProc, hwnd, msg, wParam, lParam);
	}


	constexpr uintptr_t UPDATE_MOUSE_CALL_SITE = 0x541DD7;
	const char* const MENU_FONT = "Archipelago\\fonts\\exo-2.ttf";

	void loadMenuFont(ImGuiIO& io)
	{
		HMODULE module = nullptr;
		if (!GetModuleHandleExA(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCSTR>(&loadMenuFont), &module))
		{
			return;
		}

		char path[MAX_PATH]{};
		if (GetModuleFileNameA(module, path, MAX_PATH) == 0) return;

		const std::filesystem::path fontPath = std::filesystem::path(path).parent_path() / MENU_FONT;
		std::error_code error;
		if (std::filesystem::exists(fontPath, error))
			io.Fonts->AddFontFromFileTTF(fontPath.string().c_str());
	}

	void __fastcall onUpdateMouse(CPad* pad, void*)
	{
		pad->UpdateMouse();
		if (overlayVisible)
		{
			CPad::NewMouseControllerState = CMouseControllerState{};
			CPad::OldMouseControllerState = CMouseControllerState{};
		}
	}
}

void UIManager::init()
{
	plugin::patch::RedirectCall(UPDATE_MOUSE_CALL_SITE, &onUpdateMouse);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	loadMenuFont(io);
	ImGui_ImplWin32_Init(RsGlobal.ps->window);
	ImGui_ImplDX9_Init(static_cast<IDirect3DDevice9*>(RwD3D9GetCurrentD3DDevice()));
	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowBorderSize = 0.0f;

	originalWndProc = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(RsGlobal.ps->window, GWLP_WNDPROC, 
			reinterpret_cast<LONG_PTR>(HookedWndProc)));
}

void UIManager::update()
{
	toggleWindow();
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (overlayVisible)
	{
		m_settingsMenu.draw();
	}
	
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::toggleWindow()
{
	if (m_toggleVisible.justPressed())
	{
		overlayVisible = !overlayVisible;
		ImGui::GetIO().MouseDrawCursor = overlayVisible;
	}
}

void UIManager::cleanUp()
{
	SetWindowLongPtr(RsGlobal.ps->window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalWndProc));

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}
