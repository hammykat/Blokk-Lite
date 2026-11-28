#include <BlokkLite.hpp>

// Optional: Define a custom GameObject subclass to attach update/collision logic
class Player : public GameObject {
public:
    Player(ObjectCreationData& data) : GameObject(data) {
        // Bind custom update and collision callbacks
        SetUpdateFunction(static_cast<UpdateFunctionPtr>(&Player::Update));
        SetCollisionFunction(static_cast<CollisionFunctionPtr>(&Player::OnCollision));
    }

    void Update(float delta) {
        vec2 vel = { 0.0f, 0.0f };
        const float speed = 200.0f;

        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) vel.x += speed;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) vel.x -= speed;
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) vel.y += speed;
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) vel.y -= speed;

        SetVelocity(vel);
    }

    void OnCollision(GameObject& other) {
        // Handle collision response here
    }
};

int main() {
    // 1. Initialize Window (Must happen before loading textures/sounds)
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "BlokkLite - Starter Template");
    SetTargetFPS(60);

    // 2. Initialize BlokkLite Engine
    Engine MyEngine{};

    // 3. Load Assets (Animations & Sounds)
    // MyEngine.CreateAnimation({ "assets/player_idle_0.png", "assets/player_idle_1.png" }, "Idle");
    // MyEngine.LoadSound("assets/jump.wav", "Jump", 2);

    // 4. Create GameObjects
    ObjectCreationData playerData{
        .position = { 400.0f, 225.0f },
        .velocity = { 0.0f, 0.0f },
        .frameNumber = 0,
        .animationName = "" // Replace with "Idle" once animations are loaded
    };

    Player player(playerData);

    Rect playerHitbox{ 0, 0, 32, 32 };
    player.SetCollisionRect(playerHitbox);

    // 5. Main Game Loop
    while (!WindowShouldClose()) {
        // --- UPDATE --- 
        // Any custom logic goes here

        MyEngine.ExecuteFrame(); // Run engine physics, collisions, and object updates along with rendering
    }

    // 6. Cleanup
    CloseWindow();

    return 0;
}