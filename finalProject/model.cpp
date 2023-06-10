#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>
#include <cfenv>
#include <list>

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

enum Type : int { alien = 1, opal = 2, knit = 3, shatter = 4};

class Ball {
private:
    enum Side : int { up = 1, down = 2, left = 3, right = 4 };
    float speed = 1.0f;
    glm::vec3 minArea = glm::vec3(-10.0f, 0.0f, -10.0f);
    glm::vec3 maxArea = glm::vec3(10.0f, 0.0f, 10.0f);

public:
    float size = random(0.2f, 1.0f);
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    float rot = 0.0f;
    Type type;

    int index;

    Ball(glm::vec3 playerPosition, float speed, int index, int type) {
        //it would be best to use the template above, but the function is throwing a floating point exception :/
        int choice = 1 + (rand() % 4); //rand() is considered to be the worst choice for a random num generator lol
        Side side = static_cast<Side>(choice);

        switch (side) { //choose a starting side and assign a random starting position from the specific side
        case up:
            this->position = glm::vec3(random(-10.0f, 10.0f), 0.0f, 10.0f);
            break;
        case down:
            this->position = glm::vec3(random(-10.0f, 10.0f), 0.0f, -10.0f);
            break;
        case left:
            this->position = glm::vec3(-10.0f, 0.0f, random(-10.0f, 10.0f));
            break;
        case right:
            this->position = glm::vec3(10.0f, 0.0f, random(-10.0f, 10.0f));
            break;
        }

        this->direction = glm::normalize(playerPosition - position);
        this->index = index;
        this->position += glm::vec3(0.0f, size, 0.0f);

        this->type = static_cast<Type>(type);

        //TODO add rotation
        // updatePosition(playerPosition, deltaT);
        //position must be updated outside the initializer so that the function can return a worldMatrix
    }

    bool isOutsideSquare() {
        return position.x < minArea.x || position.x > maxArea.x || position.z < minArea.z || position.z > maxArea.z;
    }

    void updatePosition(float deltaT) { //updates internal position of a ball and returns the associated worldMatrix
        glm::vec3 velocity = direction * speed / size;
        position += velocity * deltaT;
        rotation += deltaT * velocity / size;
        rot += deltaT * speed / size;
    }
    glm::mat4 getWorldMatrix() {
        glm::mat4 worldMatrix;
        glm::vec3 perpDirection = glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f));
        float angle = acos(glm::dot(perpDirection, glm::vec3(1.0f, 0.0f, 0.0f)) / (glm::length(perpDirection) * glm::length(glm::vec3(1.0f, 0.0f, 0.0f))));
        if (perpDirection.z > 0.0f) {
            angle = glm::radians(360 - glm::degrees(angle));
        }
        glm::mat4 r52 = glm::rotate(glm::mat4(1), angle, glm::vec3(0, 1, 0));
        glm::mat4 r51 = glm::rotate(glm::mat4(1), -rot, glm::vec3(1, 0, 0));
        if (!isOutsideSquare()) {
            worldMatrix = glm::translate(glm::mat4(1.0), position) * r52 * r51 * inverse(r52) * glm::scale(glm::mat4(1.0), glm::vec3(size));
        }
        else {
            worldMatrix = glm::mat4(0.0f);
        }

        return worldMatrix;
    }
};

class Wave {
    private:
        float speed;
        glm::vec3 minArea;
        glm::vec3 maxArea;

        bool isOutsideSquare(glm::vec3 position) {
            return position.x < minArea.x || position.x > maxArea.x || position.z < minArea.z || position.z > maxArea.z;
        }

    public:
        std::list<Ball> balls; //TODO usare ArrayList
        int waveSize;
        int currentBall;

        Wave(int waveSize, float speed, glm::vec3 minArea, glm::vec3 maxArea) {
            this->speed = speed;
            this->waveSize = waveSize;

            this->minArea = minArea;
            this->maxArea = maxArea;
        }

        void addBall(glm::vec3 playerPosition) {
            if(balls.size() <= waveSize) {
                int type = 1 + (rand() % 4) ;
                balls.push_back(Ball(playerPosition, speed, balls.size(), type));
            }
            std::cout << "adding: " << balls.size() << std::endl;
            
        }

        void removeOutOfBoundBalls() {
            std::list<Ball>::iterator it;
            if(waveSize >= 1) {
                for (it = balls.begin(); it != balls.end(); ++it){
                    if(isOutsideSquare(it->position)) {
                        std::cout << "deleting..."<< it->index << std::endl;
                        it = balls.erase(it);
                        if (balls.empty()) {
                            break;
                        }
                    }
                }

                int newIndex = 0;
                for(it = balls.begin(); it != balls.end(); ++it) {
                    it->index = newIndex;
                    ++newIndex;
                }
            }
        }
};