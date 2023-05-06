#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <GLFW/glfw3.h>

float M_PI = 3.14f;
int max_balls = 3;
// Define ball class
class Ball {
public:
    int sector;
    //Ball(float x, float y, float radius, float dx, float dy, float r, float g, float b) : x(x), y(y), radius(radius), dx(dx), dy(dy), r(r), g(g), b(b), bounces(0) {}
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

    void move() {
        x += dx;
        y += dy;

        int newSector = calculateSector();
        if (newSector != sector) {
            sector = newSector;
            adjustSpeed();
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
        // Calculate the sector number based on the ball's x location
        float sectorWidth = 1.0f / 3.0f;
        float normalizedX = (x + 1) / 2.0f;
        return static_cast<int>(normalizedX / sectorWidth);
    }

    void adjustSpeed() {
        // Adjust the speed based on the sector number
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
    int bounces;
};

// Define function to spawn new balls
void spawnBalls(std::vector<Ball>& balls, int& ballCount) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0.05f, 0.95f);
    std::uniform_real_distribution<float> dirDist(-0.02f, 0.02f);
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

    while (true) {
        if (ballCount < max_balls) {
            // Spawn new ball
            float x = posDist(gen);
            float y = posDist(gen);
            float dx = dirDist(gen);
            float dy = dirDist(gen);
            float r = colorDist(gen);
            float g = colorDist(gen);
            float b = colorDist(gen);
            balls.emplace_back(x, y, 0.05f, dx, dy, r, g, b, (ballCount % 3));
            
            ballCount++;
        }

        // Delay for 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Check if any ball should be removed
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
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Create window
    GLFWwindow* window = glfwCreateWindow(640, 480, "Bouncing Balls", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Make window current context
    glfwMakeContextCurrent(window);

    // Enable vsync
    glfwSwapInterval(1);

    // Initialize ball vector and count
    std::vector<Ball> balls;
    int ballCount = 0;

    // Spawn balls in a separate thread
    std::thread ballThread(spawnBalls, std::ref(balls), std::ref(ballCount));

    // Main loop
    while (!glfwWindowShouldClose(window)) {

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw balls
        for (auto it = balls.begin(); it != balls.end(); ) {
            auto& ball = *it;
            ball.draw();
            ball.move();
            if (ball.shouldRemove()) {
                it = balls.erase(it);
                ballCount--;
            }
            else {
                ++it;
            }
        }

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll for events
        glfwPollEvents();
    }

    // Wait for ball thread to finish
    ballThread.join();

    // Terminate GLFW
    glfwTerminate();

    return 0;
}