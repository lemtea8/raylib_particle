#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"
#include "time.h"

#include "raylib.h"

const float SCREEN_WIDTH = 840.0;
const float SCREEN_HEIGHT = 840.0;

const int PARTICLE_COUNT = 500000;

float randf() {
    return (float)rand() / (float)(RAND_MAX);
}

float calculateForce(float dist) {
    // prevent division by zero
    if (dist == 0) {
        return 0;
    }
    float force = 720.0 / dist;
    // avoid force being too large
    if (force > 1000) {
        force = 1000;
    }
    return force;
}

void updateParticles(float delta, Vector2* particles, Vector2* particlesSpeed, int particleCount) {
    Vector2 mousePos = GetMousePosition();
    float mouseX = mousePos.x;
    float mouseY = mousePos.y;

#pragma omp parallel for num_threads(4)
    for (int i = 0; i < particleCount; i++) {
        float x = particles[i].x, y = particles[i].y;

        // direction vector towards mouse
        float xVec = mouseX - x;
        float yVec = mouseY - y;
        // distance between particle and mouse
        float dist = xVec * xVec + yVec * yVec;

        // how many force to apply on particle
        // the closer to mouse, the larger the force (like gravity)
        float force = calculateForce(dist);

        // make force framerate-independent
        force = force * delta;

        // gradually slow down (like fraction)
        particlesSpeed[i].x *= 0.995;
        particlesSpeed[i].y *= 0.995;

        // apply new speed on particles
        particlesSpeed[i].x += force * xVec;
        particlesSpeed[i].y += force * yVec;

        // apply new position with speed
        particles[i].x = x + particlesSpeed[i].x;
        particles[i].y = y + particlesSpeed[i].y;
    }
}

void rgbImageDrawPixel(Image* dst, int x, int y, Color color) {
    if ((dst->data == NULL) || (x < 0) || (x >= dst->width) || (y < 0) || (y >= dst->height)) return;

    ((unsigned char*)dst->data)[(y * dst->width + x) * 3] = color.r;
    ((unsigned char*)dst->data)[(y * dst->width + x) * 3 + 1] = color.g;
    ((unsigned char*)dst->data)[(y * dst->width + x) * 3 + 2] = color.b;
}

void updateImageWithParticles(Image* img, Vector2* particles, int particleCount) {
    // clear image
    ImageDrawRectangle(img, 0, 0, img->width, img->height, WHITE);

#pragma omp parallel for num_threads(4)
    for (int i = 0; i < particleCount; i++) {
        // draw particle on image
        rgbImageDrawPixel(img, (int)(particles[i].x), (int)(particles[i].y), BLACK);
    }
}

int main() {
    srand(time(NULL));

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib particle");
    SetWindowState(FLAG_VSYNC_HINT);

    // avoid texture create info spamming console
    SetTraceLogLevel(LOG_ERROR);

    // store position of each particle
    Vector2* particles = malloc(sizeof(Vector2) * PARTICLE_COUNT);
    // store speed vector of each particle
    Vector2* particlesSpeed = malloc(sizeof(Vector2) * PARTICLE_COUNT);

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        particles[i].x = randf() * SCREEN_WIDTH;
        particles[i].y = randf() * SCREEN_HEIGHT;
    }

    // 3 channels: RGB
    void* buffer = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 3 * sizeof(uint8_t));
    Image img = {
        .data = buffer,
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8,
        .mipmaps = 1,
    };

    // initial particle draw
    updateImageWithParticles(&img, particles, PARTICLE_COUNT);

    bool isMouseMoved = false;

    while (!WindowShouldClose()) {
        BeginDrawing();

        // initial mouse position must be (0, 0)
        Vector2 mousePos = GetMousePosition();
        if (mousePos.x != 0 && mousePos.y != 0) {
            isMouseMoved = true;
        }

        // only move particles after first mouse move
        if (isMouseMoved) {
            // make particle speed independent from frame rate
            float delta = GetFrameTime();
            updateParticles(delta, particles, particlesSpeed, PARTICLE_COUNT);
            updateImageWithParticles(&img, particles, PARTICLE_COUNT);
        }

        // draw the particle image on screen
        Texture texture = LoadTextureFromImage(img);
        DrawTexture(texture, 0, 0, WHITE);
        DrawFPS(0, 0);

        EndDrawing();
        UnloadTexture(texture);
    }
    
    UnloadImage(img);
    free(particles);
    free(particlesSpeed);

    CloseWindow();
    return 0;
}
