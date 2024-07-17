#pragma once
//#define NOMINMAX

#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS
#endif

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <wincodec.h>

//#include <winrt/Windows.UI.Core.h>
//#include <winrt/Windows.Foundation.h>
//#include <winrt/Windows.System.Threading.h>

#define ACE_GRAPHICS_EXPORT

#ifdef ACE_GRAPHICS_EXPORT
#define ACE_GRAPHICS_API __declspec(dllexport)
#else
#define ACE_GRAPHICS_API __declspec(dllimport)
#pragma comment (lib,"AceOfHearts.Graphics.lib")
#endif

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")