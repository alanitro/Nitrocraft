#include "Camera.hpp"

Camera::Camera()
{
    CalculateProjection();

    Calculate(glm::vec3(0.0f), glm::vec2(0.0f));
}

void Camera::Calculate(glm::vec3 delta_position, glm::vec2 delta_rotation)
{
    CalculatePosition(delta_position);

    CalculateRotation(delta_rotation);

    CalculateView();

    CalculateViewProjection();
}

void Camera::SetFovY(float fovy)
{
    m_FovY = fovy;

    CalculateProjection();
}

void Camera::SetAspectRatio(float aspect)
{
    m_AspectRatio = aspect;

    CalculateProjection();
}

void Camera::SetNear(float near)
{
    m_Near = near;

    CalculateProjection();
}

void Camera::SetFar(float far)
{
    m_Far = far;

    CalculateProjection();
}

void Camera::CalculatePosition(glm::vec3 delta_position)
{
    m_Position += delta_position;
}

void Camera::CalculateRotation(glm::vec2 delta_rotation)
{
    m_Yaw += delta_rotation.x;   // Yaw
    m_Pitch += delta_rotation.y; // Pitch

    // Prevents yaw value from going over +-360 degrees
    m_Yaw = std::fmod(m_Yaw, glm::radians(360.0f));

    // Prevents pitch value from going over +-90 degrees
    if (m_Pitch > glm::radians(89.9f))
    {
        m_Pitch = glm::radians(89.9f);
    }

    if (m_Pitch < glm::radians(-89.9f))
    {
        m_Pitch = glm::radians(-89.9f);
    }

    // Rotation vector update
    m_Front.x = std::cos(m_Yaw) * std::cos(m_Pitch);
    m_Front.y = std::sin(m_Pitch);
    m_Front.z = std::sin(m_Yaw) * std::cos(m_Pitch);
    //m_Front = glm::normalize(m_Front);

    m_Right = glm::normalize(glm::cross(m_Front, glm::vec3(0.0f, 1.0f, 0.0f)));

    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

void Camera::CalculateView()
{
    m_View = glm::lookAt(m_Position, m_Position + m_Front, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::CalculateProjection()
{
    m_Projection = glm::perspective(m_FovY, m_AspectRatio, m_Near, m_Far);
}

void Camera::CalculateViewProjection()
{
    m_ViewProjection = m_Projection * m_View;
}
