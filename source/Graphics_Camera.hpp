#pragma once

#include <cstdint>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

class Camera
{
public:
    Camera();

    void Calculate(glm::vec3 delta_position, glm::vec2 delta_rotation);

    glm::vec3 GetPosition() const { return m_Position; }

    glm::vec3 GetRight() const { return m_Right; }
    glm::vec3 GetLeft()  const { return -m_Right; }
    glm::vec3 GetUp()    const { return m_Up; }
    glm::vec3 GetDown()  const { return -m_Up; }
    glm::vec3 GetFront() const { return m_Front; }
    glm::vec3 GetBack()  const { return -m_Front; }

    const glm::mat4& GetView()           const { return m_View; }
    const glm::mat4& GetProjection()     const { return m_Projection; }
    const glm::mat4& GetViewProjection() const { return m_ViewProjection; }

    void SetFovY(float fovy);
    void SetAspectRatio(float aspect);
    void SetNear(float near);
    void SetFar(float far);

private:
    float m_FovY = glm::radians(45.0f);
    float m_AspectRatio = 1.0f;
    float m_Near = 0.1f;
    float m_Far = 100.0f;

    glm::vec3 m_Position{};

    float m_Yaw = glm::radians(-90.0f);
    float m_Pitch = glm::radians(0.0f);

    glm::vec3 m_Right;
    glm::vec3 m_Up;
    glm::vec3 m_Front;

    glm::mat4x4 m_View;
    glm::mat4x4 m_Projection;
    glm::mat4x4 m_ViewProjection;

private:
    void CalculatePosition(glm::vec3 delta_position);
    void CalculateRotation(glm::vec2 delta_rotation);
    void CalculateView();
    void CalculateProjection();
    void CalculateViewProjection();
};
