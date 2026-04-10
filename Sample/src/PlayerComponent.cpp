#include "PlayerComponent.h"
#include <iostream>

void PlayerComponent::startRunning() {
	isRunning = true;
}

void PlayerComponent::stopRunning() {
    isRunning = false;
}

bool PlayerComponent::isOutOFOxygen() const {
    if (oxygen <= 0.0f) {
        return true;
    }
    else return false;
}

void PlayerComponent::OxygenDepletion() {
    oxygen -= 1.0f;
}

void PlayerComponent::TakeDamage(float amount) {
	hp -= amount;
    std::cout << "Took : " << amount << "damages. \n";
}

bool PlayerComponent::isDead() const {
    if (hp <= 0.0f) {
        return true;
    }
    else return false;
}

void PlayerComponent::OxygenReplenish(float amount) {
    oxygen += amount;
}

glm::vec3 PlayerComponent::getForward() {
	return playerTransform->GetLocalAxe<RotData::Dir::Forward>();
}

glm::vec3 PlayerComponent::getRight() {
	return playerTransform->GetLocalAxe<RotData::Dir::Right>();
}

glm::vec3 PlayerComponent::getPosition() {
    return playerTransform->GetPosition();
}

void PlayerComponent::move(const glm::vec3& direction, float deltaTime) {  
	    if (!isCrouching && !isRunning) {
            speed = 25.0f;
        }
        else if (isCrouching) {
            speed = 10.0f;
        }
        else if (isRunning) {
            speed = 40.0f;
        }
    
        position += direction * speed * deltaTime;
        playerTransform->Translate(direction * speed * deltaTime);
}

void PlayerComponent::startCrouching() {
	playerTransform->SetScale({ 1.0f, 0.5f, 1.0f });
    isCrouching = true;
}

void PlayerComponent::stopCrouching() {
	playerTransform->SetScale({ 1.0f, 1.0f, 1.0f });
    isCrouching = false;
}

glm::vec3 PlayerComponent::getRotation() {
    return playerTransform->GetRotation();
}

glm::vec3 PlayerComponent::SetRotation(const glm::vec3& newRotation) {
    playerTransform->SetRotation(newRotation);
    return playerTransform->GetRotation();
}

glm::vec3 PlayerComponent::getScale() {
    return playerTransform->GetScale();
}

glm::vec3 PlayerComponent::SetScale(const glm::vec3& newScale) {
    playerTransform->SetScale(newScale);
    return playerTransform->GetScale();
}

int PlayerComponent::getOxygen() {
    return static_cast<int>(oxygen);
}