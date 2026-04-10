#pragma once
#include <Core/TrasformComponent.h>
#include <Core/CameraComponent.h>

struct PlayerComponent {

	float speed = 25.0f;
	float hp = 100.0f;
	float oxygen = 120.0f; //base seconds of oxygen
	float verticalVelocity = 0.0f;
	int jumpCharge = 2;
	bool isGrounded = true;
	bool isRunning = false;
	bool isCrouching = false;
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;
	glm::vec3 forward;
	glm::vec3 right;
	TransformComponent* playerTransform;

	void startRunning();

	void stopRunning();

	bool isOutOFOxygen() const;

	void OxygenDepletion();
	
	int getOxygen();

	void TakeDamage(float amount);

	bool isDead() const;

	void OxygenReplenish(float amount);

	glm::vec3 getForward();

	glm::vec3 getRight();

	glm::vec3 getPosition();

	void move(const glm::vec3& direction, float deltaTime);

	void startCrouching();
	
	void stopCrouching();

	glm::vec3 getRotation();

	glm::vec3 SetRotation(const glm::vec3& newRotation);

	glm::vec3 getScale();

	glm::vec3 SetScale(const glm::vec3& newScale);


	void Init(TransformComponent* selfTransform) {
		playerTransform = selfTransform;
		position = selfTransform->GetPosition();
		rotation = selfTransform->GetRotation();
		scale = selfTransform->GetScale();
		forward = selfTransform->GetLocalAxe<RotData::Dir::Forward>();
		right = selfTransform->GetLocalAxe<RotData::Dir::Right>();
		speed = 25.0f;
		hp = 100.0f;
		oxygen = 120.0f;
		verticalVelocity = 0.0f;
		jumpCharge = 2;
		isGrounded = true;
		isRunning = false;
	}

};
