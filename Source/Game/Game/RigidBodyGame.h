#pragma once

#include "Game/GameSample.h"
#include <unordered_map>
#include "Shapes/Shape.h"

class RigidBodyGame final : public DX12Library::GameSample
{
public:
	RigidBodyGame(_In_ PCWSTR pszGameName);
	~RigidBodyGame();

	virtual void InitDevice(void);
	virtual void CleanupDevice(void);
	virtual void HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime);
	virtual void Update(_In_ FLOAT deltaTime);
	virtual void Render(void);

	HRESULT AddShape(const std::wstring shapeName, std::shared_ptr<DX12Library::Shape> shape);

	void CollectCollisionPairs(void);
	void SimulatePhysics(void);

private:
	// Pipeline objects.
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12Resource> m_renderTargets[FRAMECOUNT];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize = 0;

	// App resources.
	ComPtr<ID3D12Resource> m_depthBuffer;
	ConstantBuffer m_constantBuffer;
	std::unordered_map<std::wstring, std::shared_ptr<DX12Library::Shape>> m_shapes;
	std::unordered_map<std::wstring, std::pair<std::wstring, std::wstring>> m_collisionPairs;

	// Synchronization objects.
	UINT m_frameIndex = 0;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;
};