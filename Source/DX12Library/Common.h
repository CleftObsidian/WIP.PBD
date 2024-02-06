//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "d3dx12.h"

#include <string>
#include <wrl.h>
#include <shellapi.h>

#include <DirectXColors.h>
#include "Resource.h"

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace)

constexpr LPCWSTR PSZ_TITLE = L"PBD Simulation";

using namespace DirectX;

struct DirectionsInput
{
    BOOL bFront;
    BOOL bLeft;
    BOOL bBack;
    BOOL bRight;
    BOOL bUp;
    BOOL bDown;
};

struct MouseRelativeMovement
{
    LONG X;
    LONG Y;
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 color;
};

struct ConstantBuffer
{
	XMMATRIX World = XMMatrixIdentity();
	XMMATRIX View;
	XMMATRIX Projection;
	XMFLOAT3 CameraPos;
};

struct BasicMeshEntry
{
	BasicMeshEntry()
		: uNumIndices(0u)
		, uBaseVertex(0u)
		, uBaseIndex(0u)
	{
	}

	UINT uNumIndices;
	UINT uBaseVertex;
	UINT uBaseIndex;
};

// Number of iteration for solving constraints
static constexpr size_t SOLVER_ITERATION = 2;

constexpr XMVECTORF32 GRAVITY = { 0.0f, -9.8f, 0.0f, 0.0f };