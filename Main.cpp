#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <functional>
#include <GLFW/glfw3.h>
#include <cmath>

float M_PI = 3.14f;
int max_balls = 15;
int middleBalls = 0;

class Ball {
public:
    Ball(float x, float y, float radius, float dx, float dy, float r, float g, float b, int sector)
        : x(x), y(y), radius(radius), dx(dx), dy(dy), r(r), g(g), b(b), bounces(0), sector(sector) {}

    void draw() {
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(r, g, b);
        glVertex2f(x, y);
        for (int i = 0; i <= 360; i++) {
            glVertex2f(x + radius * cos(i * M_PI / 180.0), y + radius * sin(i * M_PI / 180.0));
        }
        glEnd();
    }

    void move(Ball* ball) {
        if (shouldRemove()) {
            return;
        }
        x += dx;
        y += dy;

        int newSector = calculateSector();
        if (newSector != sector) {
            sector = newSector;
            adjustSpeed();

            //if (newSector == 1) {
            //    while (middleBalls >= 2) {
            //        std::this_thread::sleep_for(std::chrono::seconds(1));
            //    }
            //    middleBalls++;
            //}
            //else
            //{
            //    middleBalls--;
            //}
        }

        if (x < -1 + radius || x > 1 - radius) {
            dx = -dx;
            bounces++;
        }
        if (y < -1 + radius || y > 1 - radius) {
            dy = -dy;
            bounces++;
        }
    }

    int calculateSector() {
        float sectorWidth = 1.0f / 3.0f;
        float normalizedX = (x + 1) / 2.0f;
        return static_cast<int>(normalizedX / sectorWidth);
    }

    void adjustSpeed() {
        switch (sector) {
        case 0:
            dx *= 0.8f;
            dy *= 0.8f;
            break;
        case 1:
            dx *= 1.2f;
            dy *= 1.2f;
            break;
        case 2:
            dx *= 0.9f;
            dy *= 0.9f;
            break;
        default:
            break;
        }
    }

    bool shouldRemove() {
        return bounces >= 6;
    }

    float x, y, radius, dx, dy, r, g, b;
    int bounces, sector;
};

int calculateSector(int x) {
    float sectorWidth = 1.0f / 3.0f;
    float normalizedX = (x + 1) / 2.0f;
    return static_cast<int>(normalizedX / sectorWidth);
}

void spawnBalls(std::vector<Ball>& balls, int& ballCount) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0.05f, 0.95f);
    std::uniform_real_distribution<float> dirDist(-0.02f, 0.02f);
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

    while (true) {
        if (ballCount < max_balls) {
            float x = posDist(gen);
            float y = posDist(gen);
            float dx = dirDist(gen);
            float dy = dirDist(gen);
            float r = colorDist(gen);
            float g = colorDist(gen);
            float b = colorDist(gen);
            balls.emplace_back(x, y, 0.05f, dx, dy, r, g, b, calculateSector(x));
            ballCount++;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        auto it = balls.begin();
        while (it != balls.end()) {
            auto& ball = *it;
            if (ball.shouldRemove()) {
                it = balls.erase(it);
                ballCount--;
            }
            else {
                ++it;
            }
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Bouncing Balls", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    std::vector<Ball> balls;
    int ballCount = 0;

    std::thread ballThread(spawnBalls, std::ref(balls), std::ref(ballCount));

    std::vector<std::thread> ballThreads;
    for (auto& ball : balls) {
        ballThreads.emplace_back(std::bind(&Ball::move, &ball, &ball));
    }

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto& ball : balls) {
            ball.draw();
        }

        // Draw sector lines
        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        for (int i = 0; i < 3; i++) {
            float sectorWidth = 2.0f / 3;
            float sectorStartX = -1.0f + i * sectorWidth;
            glVertex2f(sectorStartX, -1.0f);
            glVertex2f(sectorStartX, 1.0f);
        }
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();

        for (auto& thread : ballThreads) {
            thread.join();
        }

        ballThreads.clear();
        for (auto& ball : balls) {
            ballThreads.emplace_back(std::bind(&Ball::move, &ball, &ball));
        }
    }

    ballThread.join();

    glfwTerminate();

    return 0;
}
