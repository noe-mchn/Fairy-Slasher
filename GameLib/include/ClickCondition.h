#pragma once
#include "Core/TrasformComponent.h"
#include "../../GameLib/include/inventory.h"

class ClickCondition {
public:
	ClickCondition() = default;
	~ClickCondition() = default;
	void update(TransformComponent Cube, TransformComponent cam)
	{
		auto forward = cam.GetLocalAxe<RotData::Dir::Forward>();
		auto pos = cam.GetPosition();

		if (glm::length(forward) < 0.0001f)
			return;
		glm::vec3 dir = glm::normalize(forward);
		float distance = maxDistance * 0.33f;
		glm::vec3 cubePos = Cube.GetPosition();

		glm::vec3 center = pos + dir * distance;
		float radius = maxDistance * 0.5f;
		glm::vec3 diff = cubePos - center;
		float distSq = glm::dot(diff, diff);

		if (distSq <= radius * radius) {
			printf("hited\n");
		}
		isClicked = false;
	}
	void isclicked() { isClicked = true; };
	bool isClicked = false;
	float maxDistance = 5.0f;
};