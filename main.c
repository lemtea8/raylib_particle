#include "stdbool.h"
#include "stdlib.h"
#include "time.h"

#include "raylib.h"

float min(float x, float y) {
	if (x < y) {
		return x;
	}
	return y;
}

float calStrength(float dist) {
    float str = 5.0 / dist;
    // avoid strength too large
	str = min(str, 1000);
    return str;
}

void particlesFollowMouse(Vector2 mousePos, int particleCount, Vector2* particles, Vector2* particlesSpeed) {
    float mouseX = mousePos.x;
    float mouseY = mousePos.y;

    for (int i = 0; i < particleCount; i++) {
        float x = particles[i].x, y = particles[i].y;

        // direction vector towards mouse
        float deltaX = mouseX - x;
        float deltaY = mouseY - y;
        // distance between particle and mouse
        float dist = deltaX * deltaX + deltaY * deltaY;

        // how many force to apply on particle
        // the closer to mouse, the larger the force (like gravity)
        float strength = calStrength(dist);

        // gradually slow down (like fraction)
        particlesSpeed[i].x *= 0.995;
        particlesSpeed[i].y *= 0.995;

        // apply new speed on particles
        particlesSpeed[i].x += strength * deltaX;
        particlesSpeed[i].y += strength * deltaY;

        particles[i].x = x + particlesSpeed[i].x;
        particles[i].y = y + particlesSpeed[i].y;
    }
}

float randf() {
	return (float)rand()/(float)(RAND_MAX);
}

int main() {
	srand(time(NULL));

    InitWindow(840, 840, "raylib particle");
    // SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowState(FLAG_VSYNC_HINT);

    const int particleCount = 200000;

    Vector2 particles[particleCount];
    Vector2 particlesSpeed[particleCount];

    for (int i = 0; i < particleCount; i++) {
        particles[i].x = randf() * 840.0;
        particles[i].y = randf() * 840.0;
    }

    bool isMouseMoved = false;

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(WHITE);
        DrawFPS(0, 0);

        // Initial mouse position must be (0, 0)
        Vector2 pos = GetMousePosition();
        if (pos.x != 0 && pos.y != 0) {
            isMouseMoved = true;
        }

        if (isMouseMoved) {
            particlesFollowMouse(pos, particleCount, particles, particlesSpeed);
        }
        for (int i = 0; i < particleCount; i++) {
            DrawPixelV(particles[i], BLACK);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
