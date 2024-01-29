#include "Game/Game.h"
#include "Shapes/Cube.h"
#include "Shapes/Plane.h"

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    std::unique_ptr<Game> game = std::make_unique<Game>(PSZ_TITLE);

	XMVECTOR position = XMVectorSet(0.0f, 20.0f, 0.0f, 0.0f);
	std::shared_ptr<DX12Library::Cube> cube1 = std::make_shared<DX12Library::Cube>(position);
	ThrowIfFailed(game->AddShape(L"Cube1", cube1));

	position = XMVectorSet(5.0f, 10.0f, 0.0f, 0.0f);
	std::shared_ptr<DX12Library::Cube> cube2 = std::make_shared<DX12Library::Cube>(position);
	ThrowIfFailed(game->AddShape(L"Cube2", cube2));

	position = XMVectorSet(-5.0f, 5.0f, 0.0f, 0.0f);
	std::shared_ptr<DX12Library::Cube> cube3 = std::make_shared<DX12Library::Cube>(position);
	ThrowIfFailed(game->AddShape(L"Cube3", cube3));

	position = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	std::shared_ptr<DX12Library::Plane> plane = std::make_shared<DX12Library::Plane>(position);
	ThrowIfFailed(game->AddShape(L"Plane", plane));

	ThrowIfFailed(game->Initialize(hInstance, nCmdShow));

    return game->Run();
}