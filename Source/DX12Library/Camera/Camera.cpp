#include "Camera.h"

namespace DX12Library
{
	Camera::Camera(_In_ const XMVECTOR& position)
        : m_yaw(0.0f)
        , m_pitch(0.0f)
        , m_moveLeftRight(0.0f)
        , m_moveBackForward(0.0f)
        , m_moveUpDown(0.0f)
        , m_travelSpeed(5.0f)
        , m_rotationSpeed(1.0f)
        , m_cameraForward(DEFAULT_FORWARD)
        , m_cameraRight(DEFAULT_RIGHT)
        , m_cameraUp(DEFAULT_UP)
        , m_eye(position)
        , m_at(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f))
        , m_up(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
        , m_rotation(XMMatrixIdentity())
        , m_view(XMMatrixLookAtLH(m_eye, m_at, m_up))
	{
	}

    const XMVECTOR& Camera::GetEye() const
    {
        return m_eye;
    }

    const XMVECTOR& Camera::GetAt() const
    {
        return m_at;
    }

    const XMVECTOR& Camera::GetUp() const
    {
        return m_up;
    }

    const XMMATRIX& Camera::GetView() const
    {
        return m_view;
    }

    void Camera::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        // Determine the amount of movement
        if (directions.bBack)
        {
            m_moveBackForward -= m_travelSpeed * deltaTime;
        }
        if (directions.bFront)
        {
            m_moveBackForward += m_travelSpeed * deltaTime;
        }
        if (directions.bLeft)
        {
            m_moveLeftRight -= m_travelSpeed * deltaTime;
        }
        if (directions.bRight)
        {
            m_moveLeftRight += m_travelSpeed * deltaTime;
        }
        if (directions.bUp)
        {
            m_moveUpDown += m_travelSpeed * deltaTime;
        }
        if (directions.bDown)
        {
            m_moveUpDown -= m_travelSpeed * deltaTime;
        }

        // Determines the rotation
        m_yaw += static_cast<FLOAT>(mouseRelativeMovement.X * m_rotationSpeed * deltaTime);
        static FLOAT newPitch = 0.0f;
        newPitch = m_pitch + static_cast<FLOAT>(mouseRelativeMovement.Y * m_rotationSpeed * deltaTime);

        // Limit pitch range(-pi/2, pi/2)
        if (newPitch < -XM_PIDIV2 || newPitch > XM_PIDIV2)
        {
            // empty
        }
        else
        {
            m_pitch = newPitch;
        }

        Update(deltaTime);
    }

    void Camera::Update(_In_ FLOAT deltaTime)
    {
        UNREFERENCED_PARAMETER(deltaTime);

        // Rotation Matrix
        m_rotation = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);
        m_at = XMVector3TransformCoord(DEFAULT_FORWARD, m_rotation);
        m_at = XMVector3Normalize(m_at);

        // Update forward / right / up vector
        XMMATRIX RotateYTempMatrix = XMMatrixRotationY(m_yaw);
        m_cameraForward = XMVector3TransformCoord(DEFAULT_FORWARD, RotateYTempMatrix);
        m_cameraRight = XMVector3TransformCoord(DEFAULT_RIGHT, RotateYTempMatrix);
        m_cameraUp = XMVector3TransformCoord(m_cameraUp, RotateYTempMatrix);

        // New eye, at, up
        m_eye += m_moveBackForward * m_cameraForward;
        m_eye += m_moveLeftRight * m_cameraRight;
        m_eye += m_moveUpDown * m_cameraUp;

        m_at += m_eye;
        m_up = m_cameraUp;

        // Reset movement
        m_moveBackForward = 0.0f;
        m_moveLeftRight = 0.0f;
        m_moveUpDown = 0.0f;

        // Determine the view matrix
        m_view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    }
}