# Using the engine

Blokk Lite is designed to be easy to use, yet powerful. The engine has a central manager for everything, including sounds, animations, rendering, and object management.

## Initialization

Blokk Lite is meant to be used with Raylib, a simple optimized library wrapper over OpenGL. 

To start, you only have to:
```cpp
#include <BlokkLite.hpp>
```

### Window

Create a window using raylib first:
```cpp
void InitWindow(int width, int height, const char *title);
```

Then, set the target fps:
```cpp
void SetTargetFPS(int fps);
```

### Engine

Now, you can create an instance of the engine that will handle and update the GameObjects. The class is `Engine`,  and the constructor takes no arguments. 
```cpp
Engine MyEngine{};
```

## Creating the game loop

After creating the window and initializing the engine, you can create your game's loop. Inside the `main()` function, add a `while()` loop that checks if not `WindowShouldClose()`:
```
int main() {

    while(!WindowShouldClose()) {
        
    }

}
```

The `while()` loop will run at the target fps per second, so put any game logic in there that needs to be calculated every frame.

Then, after the `while` loop, add a `CloseWindow` function to de-initialize the window and OpenGL context.

So the entire thing might look like this:

```
int main() {

    while(!WindowShouldClose()) {
        
    }

    CloseWindow();
}
```

## Using the Engine

The engine is the core manager of everything. It handles object updates, data, sounds, animations, and rendering.

> Please note that these are all member functions of the `Engine` class

### Animations

You can create an animation using some different functions:
```cpp
// Creates an animation called 'name' and loads every file in 'paths' as a frame, in order
void CreateAnimation(const std::vector<std::filesystem::path>& paths, const std::string& name);

// Creates an animation called 'name' with no frames. Use AddAnimationFrame() to fill it
void CreateEmptyAnimation(const std::string& name);

// Loads the file at 'path' and adds it as the last frame of the animation called 'name'
void AddAnimationFrame(const std::string& name, const std::filesystem::path& path);
```

Remember that textures can only be loaded **after raylib's InitWindow() function**!

You can also clear all animations, which is automatically done by the engine in the constructor:
```cpp
void ClearAnimations();
```

### Sounds

You can load a sound from a filepath using the function:
```cpp
void LoadSound(const std::filesystem::path& path, const std::string& name, uint32_t maxConcurrentInstances = 1);
```
This function takes the name of the sound, along with the name of the sound. 
The third parameter represents the max number of instances of the sound that can be ran at the same time.

Playing a sound takes the name of the sound and the volume (defaulting to 100):
```cpp
void PlaySoundEffect(const std::string& name, float volume = 100.0f);
```

You can stop all instances of a sound:
```cpp
void StopSound(const std::string& name);
```

And you can clear all the sounds, which is automatically done by the engine in the destructor:
```cpp
void ClearSounds();
```

## Creating & Using GameObjects

Creating a game object is just creating a new instamce of the `GameObject` class, which handles synchronization and connection with the engine for you. When creating a new game object, you need to **ensure that an instance of the `Engine` class has been created** or else it will throw an error, because there is no engine to connect to. 

The GameObject constructor takes the argument of an instance of the `ObjectCreationData` struct, which defines what the object's starting properties are:
```cpp
struct ObjectCreationData {
	const vec2 position = { 0, 0 }, velocity = { 0, 0 }; // velocity and position of the object
	const uint32_t frameNumber = UINT32_MAX;
	const std::string& animationName{ };
};
```

### Collisions

Setting the collision rectangle requires creating a new instance of the `Rect` struct, a basic structure for collision rectangles:
```cpp
struct Rect {
	int x, y; // Top left 
	int w, h; // Dimensions
};
```
This rect defines the rectangle the object uses for collisions. If an object collides with another object, it will not be allowed to move into the other object's hitbox and will instead smoothly make each object stop. 

Every object has a collision function - A function that runs when it collides with another object:
```cpp
void GameObject::OnCollision(GameObject* object);
```
The parameter is the object that it collided with.

You can set the function as the object's collision function using:
```cpp
void SetCollisionFunction(CollisionFunctionPtr func) {
	CollisionFunction = func;
	CollisionFunctionSet = (func != nullptr);
}
```
Where:
```cpp
using CollisionFunctionPtr = void(GameObject::*)(GameObject& Object);
```

Then, the engine will automatically call the function if its set and the object collides with another object.

### Animations

Every object has an animation. After loading the animations using the Engine, you cnan easily set the object's animation or frame number:
```cpp
void SetFrameNumber(uint32_t num);
void SetAnimation(const std::string& name);
```

### All Functions 

| Function Signature | Description & Use |
| :--- | :--- |
| `GameObject(ObjectCreationData& creationData)` | Constructor that initializes the object and registers it with the active engine instance. |
| `void SetVelocity(vec2 vel)` | Sets the object's X and Y velocity values. |
| `[[nodiscard]] vec2 GetVelocity() const` | Returns the object's current X and Y velocity as a `vec2`. |
| `void SetPosition(vec2 pos)` | Sets the object's X and Y position coordinates. |
| `[[nodiscard]] vec2 GetPosition() const` | Returns the object's current X and Y position as a `vec2`. |
| `void SetCollisionRect(Rect& rect)` | Updates the collision rectangle if it exists, or tells the engine to register a new one. |
| `void SetFrameNumber(uint32_t num)` | Sets the current frame number, ensuring it is within bounds for the active animation. |
| `void SetAnimation(const std::string& name)` | Changes the active animation by name, resetting to frame 0 if the current frame is out of bounds (throws if invalid). |


## Code example

You can also find this [here](Template.cpp)

```cpp
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
        if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) vel.x -= speed;
        if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) vel.y += speed;
        if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) vel.y -= speed;

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

    Rect playerHitbox{ 400, 225, 32, 32 };
    player.SetCollisionRect(playerHitbox);

    // 5. Main Game Loop
    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        // --- UPDATE ---
        // MyEngine.Update(delta); // Run engine physics, collisions, and object updates

        // --- DRAW ---
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // MyEngine.Render(); // Render active animations/sprites
            DrawText("BlokkLite Engine Initialized!", 20, 20, 20, DARKGRAY);

        EndDrawing();
    }

    // 6. Cleanup
    CloseWindow();

    return 0;
}
```