#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>
#include <cfenv>

/*
struct wave {
	ball balls[20];
	int increment;
	int waveSize;
};
*/

template<typename T>
T random(T range_from, T range_to) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<T> dis(range_from, range_to);
    return dis(gen);
}



class Ball {
    private:
        enum Side: int {up = 1, down = 2, left = 3, right = 4};

    public:
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 direction = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        float speed = 1.0f;
        int index = 0;

        Ball(glm::vec3 playerPosition, float deltaT) {
            //it would be best to use the template above, but the function is throwing a floating point exception :/
            int choice = 1 + (rand() % 4); //rand() is considered to be the worst choice for a random num generator lol
            Side side = static_cast<Side>(choice);

            switch (side) { //choose a starting side and assign a random starting position from the specific side
                case up:
                    position = glm::vec3(random(-5.0f, 5.0f), 0.0f, 5.0f);
                    break;
                case down:
                    position = glm::vec3(random(-5.0f, 5.0f), 0.0f, -5.0f);
                    break;
                case left:
                    position = glm::vec3(-5.0f, 0.0f, random(-5.0f, 5.0f));
                    break;
                case right:
                    position = glm::vec3(5.0f, 0.0f, random(-5.0f, 5.0f));
                    break;
            }

            direction = glm::normalize(playerPosition - position);

            //TODO add rotation
            // updatePosition(playerPosition, deltaT);
            //position must be updated outside the initializer so that the function can return a worldMatrix
        } 
        
        glm::mat4 updatePosition(float deltaT) {
            glm::vec3 velocity = direction * speed;
            position += velocity * deltaT;

            glm::vec3 right = glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f));
		    glm::vec3 newUp = glm::cross(right, direction);

            glm::mat4 worldMatrix = glm::mat4(
                glm::vec4(right, 0.0f),
                glm::vec4(newUp, 0.0f),
                glm::vec4(direction, 0.0f),
                glm::vec4(position, 1.0f)
		    );

            return worldMatrix;
        }
};