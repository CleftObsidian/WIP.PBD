#include "Game/Game.h"
#include "Game/RigidBodyGame.h"
#include "Shapes/Cube.h"
#include "Shapes/Sphere.h"
#include "Shapes/Plane.h"
#include "Shapes/RigidBodySphere.h"

#define RIGIDBODY_SIMULATION
//#undef RIGIDBODY_SIMULATION

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef RIGIDBODY_SIMULATION
	std::unique_ptr<RigidBodyGame> game = std::make_unique<RigidBodyGame>(PSZ_TITLE);
	
	constexpr float staticFrictionCoefficient = 0.5f;
	constexpr float dynamicFrictionCoefficient = 0.4f;
	constexpr float restitutionCoeftticient = 0.1f;
	
	XMVECTOR position = XMVectorZero();
	XMVECTOR rotation = XMQuaternionIdentity();
	XMVECTOR scale = XMVectorZero();

	{
		float sphereRadius = 1.0f;
		position = XMVectorSet(0.0f, 20.0f, 0.0f, 0.0f);
		scale = XMVectorSet(sphereRadius, sphereRadius, sphereRadius, 0.0f);
		bool bIsFixed = false;

		std::vector<Collider> colliders;
		Collider colliderSphere = CreateColliderSphere(sphereRadius);
		colliders.push_back(colliderSphere);

		std::shared_ptr<DX12Library::RigidBodySphere> sphere = std::make_shared<DX12Library::RigidBodySphere>(position, rotation, scale, 1.0f,
			colliders, staticFrictionCoefficient, dynamicFrictionCoefficient, restitutionCoeftticient, bIsFixed);
		game->AddShape(sphere);
	}
	
	{
		float sphereRadius = 5.0f;
		position = XMVectorSet(0.1f, 0.0f, 0.0f, 0.0f);
		scale = XMVectorSet(sphereRadius, sphereRadius, sphereRadius, 0.0f);
		bool bIsFixed = true;

		std::vector<Collider> colliders;
		Collider colliderSphere = CreateColliderSphere(sphereRadius);
		colliders.push_back(colliderSphere);

		std::shared_ptr<DX12Library::RigidBodySphere> sphere = std::make_shared<DX12Library::RigidBodySphere>(position, rotation, scale, 1.0f,
			colliders, staticFrictionCoefficient, dynamicFrictionCoefficient, restitutionCoeftticient, bIsFixed);
		game->AddShape(sphere);
	}

#else
    std::unique_ptr<Game> game = std::make_unique<Game>(PSZ_TITLE);

	XMVECTOR position = XMVectorZero();
	if (false)
	{
		position = XMVectorSet(0.0f, 5.0f, 0.0f, 0.0f);
		std::shared_ptr<DX12Library::Cube> cube1 = std::make_shared<DX12Library::Cube>(position);
		ThrowIfFailed(game->AddShape(L"Cube1", cube1));

		position = XMVectorSet(0.0f, 3.0f, 0.0f, 0.0f);
		std::shared_ptr<DX12Library::Cube> cube2 = std::make_shared<DX12Library::Cube>(position);
		ThrowIfFailed(game->AddShape(L"Cube2", cube2));

		position = XMVectorSet(-2.0f, 3.0f, 0.0f, 0.0f);
		std::shared_ptr<DX12Library::Cube> cube3 = std::make_shared<DX12Library::Cube>(position);
		ThrowIfFailed(game->AddShape(L"Cube3", cube3));

		position = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		std::shared_ptr<DX12Library::Cube> cube4 = std::make_shared<DX12Library::Cube>(position);
		ThrowIfFailed(game->AddShape(L"Cube4", cube4));
	}
	else
	{
		position = XMVectorSet(0.0f, 20.0f, 0.0f, 0.0f);
		std::shared_ptr<DX12Library::Sphere> sphere1 = std::make_shared<DX12Library::Sphere>(position);
		ThrowIfFailed(game->AddShape(L"Sphere1", sphere1));

		position = XMVectorSet(0.0f, 3.0f, 0.0f, 0.0f);
		std::shared_ptr<DX12Library::Sphere> sphere2 = std::make_shared<DX12Library::Sphere>(position);
		ThrowIfFailed(game->AddShape(L"Sphere2", sphere2));

		position = XMVectorSet(-2.0f, 3.0f, 0.0f, 0.0f);
		std::shared_ptr<DX12Library::Sphere> sphere3 = std::make_shared<DX12Library::Sphere>(position);
		ThrowIfFailed(game->AddShape(L"Sphere3", sphere3));

		position = XMVectorSet(1.0f, 1.0f, 0.0f, 0.0f);
		std::shared_ptr<DX12Library::Sphere> sphere4 = std::make_shared<DX12Library::Sphere>(position);
		ThrowIfFailed(game->AddShape(L"Sphere4", sphere4));
	}
	
	position = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	std::shared_ptr<DX12Library::Plane> plane = std::make_shared<DX12Library::Plane>(position);
	ThrowIfFailed(game->AddShape(L"Plane", plane));
#endif // RIGIDBODY_SIMULATION
	
	ThrowIfFailed(game->Initialize(hInstance, nCmdShow));

    return game->Run();
}