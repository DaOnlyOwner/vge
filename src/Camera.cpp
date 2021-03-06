#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"
#include <algorithm>


// This is from my bitbucket repo: https://bitbucket.org/DaTrueOwner/global_illumination/src/WithBricks2/
Camera::Camera(float aspect)
{
	SetEyePos({ 0,0,0 });
	SetPerspectiveProj(87.f, aspect, 0, 1000);
}

glm::mat4 Camera::GetViewMatrix()
{
	if (view_dirty) {
		view = glm::lookAt(eye, eye + forward, WorldUp);
		view_dirty = false;
	}
	return view;
}


glm::mat4 Camera::GetProjMatrix() const
{
	return proj;
}

void Camera::Move(const glm::vec3& delta)
{
	if (abs(delta.x) <= 0.00000001 && abs(delta.y) <= 0.00000001 && abs(delta.z) <= 0.00000001) return;
	float len = glm::length(delta);
	// delta is in local space -> transform it to view space
	// compute right:
	auto right = glm::cross(forward, WorldUp);
	// compute up:
	auto up = glm::cross(right, forward);
	// compute new coord system
	glm::mat3 viewCoord(right, forward, up);
	eye += glm::normalize(viewCoord * delta) * len;
	view_dirty = true;
}

void Camera::RotateEye(float phi, float theta)
{
	glm::vec3 center = forward;
	float thetaNew = acos(center.y) + glm::radians(theta);
	float phiNew = atan2(center.z, center.x) + glm::radians(phi);


	thetaNew = std::clamp(thetaNew, 0.f, 3.14f);
	phiNew = fmod(phiNew, (3.14f * 2));

	forward.x = sin(thetaNew) * cos(phiNew);
	forward.z = sin(thetaNew) * sin(phiNew);
	forward.y = cos(thetaNew);
	forward = normalize(forward);
	view_dirty = true;
}

void Camera::LookAt(const glm::vec3& center)
{
	forward = glm::normalize(center - eye);
	assert((forward != glm::vec3{ 0, 0, 0 }));
	view_dirty = true;
}

void Camera::SetEyePos(const glm::vec3& eye_pos)
{
	eye = eye_pos;
	view_dirty = true;
}

glm::vec3 Camera::GetEyePos() const
{
	return eye;
}

glm::vec3 Camera::GetForward() const
{
	return forward;
}