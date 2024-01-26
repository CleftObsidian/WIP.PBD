#pragma once

#include "DXSampleHelper.h"

namespace DX12Library
{
    class Camera
    {
    public:
        Camera() = delete;
        Camera(_In_ const XMVECTOR& position);
        Camera(const Camera& other) = delete;
        Camera(Camera&& other) = delete;
        Camera& operator=(const Camera& other) = delete;
        Camera& operator=(Camera&& other) = delete;
        virtual ~Camera() = default;

        const XMVECTOR& GetEye() const;
        const XMVECTOR& GetAt() const;
        const XMVECTOR& GetUp() const;
        const XMMATRIX& GetView() const;

        virtual void HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime);
        virtual void Update(_In_ FLOAT deltaTime);
    protected:
        static constexpr const XMVECTORF32 DEFAULT_FORWARD = { 0.0f, 0.0f, 1.0f, 0.0f };
        static constexpr const XMVECTORF32 DEFAULT_RIGHT = { 1.0f, 0.0f, 0.0f, 0.0f };
        static constexpr const XMVECTORF32 DEFAULT_UP = { 0.0f, 1.0f, 0.0f, 0.0f };

        FLOAT m_yaw;
        FLOAT m_pitch;

        FLOAT m_moveLeftRight;
        FLOAT m_moveBackForward;
        FLOAT m_moveUpDown;

        FLOAT m_travelSpeed;
        FLOAT m_rotationSpeed;

        BYTE m_padding[20]; // struct alignment

        XMVECTOR m_cameraForward;
        XMVECTOR m_cameraRight;
        XMVECTOR m_cameraUp;

        XMVECTOR m_eye;
        XMVECTOR m_at;
        XMVECTOR m_up;

        XMMATRIX m_rotation;
        XMMATRIX m_view;
    };
}