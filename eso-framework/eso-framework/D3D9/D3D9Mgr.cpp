#include "D3D9Mgr.h"
#include "../EsoMgr.h"
#include <detours.h>
#include <sstream>

void D3D9Mgr::Initialize()
{

	EsoMgr* esoMgr = EsoMgr::GetSingleton();
	ZoRenderManager* renderManager = esoMgr->GetRenderManager();

	if (renderManager)
	{

		if (renderManager->GetRenderApiType() == ZoRenderManager::RENDER_API::D3D9)
		{

			ZoRenderDeviceD3D9* renderDeviceD3D9 = renderManager->GetRenderDeviceD3D9();

			if (renderDeviceD3D9)
			{

				IDirect3DDevice9* d3d9Device = renderDeviceD3D9->GetDevice();

				if (d3d9Device)
				{
					
					DWORD* vTable = *(DWORD**)d3d9Device;
					s_resetAddress = vTable[16];
					s_presentAddress = vTable[17];

					oPresent = (Present_t)DetourFunction((PBYTE)s_presentAddress, (PBYTE)MyPresent);
					oReset = (Reset_t)DetourFunction((PBYTE)s_resetAddress, (PBYTE)MyReset);

				}
				else
				{
					MessageBoxA(NULL, "Couldn't grab IDirect3DDevice9", "Error", MB_ICONERROR);
				}

			}
			else
			{
				MessageBoxA(NULL, "Couldn't grab ZoRenderDeviceD3D9", "Error", MB_ICONERROR);
			}

		}
		else
		{
			MessageBoxA(NULL, "Game must run in DirectX9 mode !", "Error", MB_ICONERROR);
		}

	}
	else
	{
		MessageBoxA(NULL, "Couldn't grab ZoRenderManager !", "Error", MB_ICONERROR);
	}

}

void D3D9Mgr::Shutdown()
{

	if (s_presentAddress)
	{
		DetourRemove((PBYTE)oPresent, (PBYTE)MyPresent);
		s_presentAddress = NULL;
	}

	if (s_resetAddress)
	{
		DetourRemove((PBYTE)oReset, (PBYTE)MyReset);
		s_resetAddress = NULL;
	}

}

HRESULT __stdcall D3D9Mgr::MyPresent(LPDIRECT3DDEVICE9* pDevice, const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
	OnFrame();
	return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT __stdcall D3D9Mgr::MyReset(LPDIRECT3DDEVICE9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	return oReset(pDevice, pPresentationParameters);
}

void D3D9Mgr::OnFrame()
{
	EsoMgr::GetSingleton()->OnFrame();
}

DWORD D3D9Mgr::s_resetAddress;
DWORD D3D9Mgr::s_presentAddress;
Present_t D3D9Mgr::oPresent = NULL;
Reset_t D3D9Mgr::oReset = NULL;
