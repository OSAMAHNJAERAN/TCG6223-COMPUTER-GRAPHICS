#ifndef SLIME_HPP
#define SLIME_HPP

#include <GL/glut.h>
#include <vector>
#include <cmath>

class Slime {
private:
    // Character stats
    float health;
    float maxHealth;
    bool isShieldActive;
    float shieldDuration;
    float shieldCooldown;

    // Position and movement
    float x, y, z;
    float targetX, targetZ;
    bool isMoving;
    float moveSpeed;
    float stepDistance;

    float rotationY = 0.0f; // Y-axis rotation

    // Animation states
    enum AnimationState {
        IDLE,
        WALKING,
        THROWING,
        SHIELDING,
        HURT,
        BOUNCING
    };
    AnimationState currentState;
    float animationTime;
    float stateTime;

    // Visual effects
    float bounceOffset;
    float squishFactor;
    float glowIntensity;
    bool isFlashing;
    float flashTime;

    // Attack properties - Slime Bomb
    float attackDamage;
    float attackRange;
    float attackCooldown;
    bool isThrowing;
    float throwAnimationTime;

    // Slime bomb projectiles
    struct SlimeBomb {
        float x, y, z;
        float vx, vy, vz;
        float targetX, targetZ;
        float life;
        float maxLife;
        float size;
        bool isActive;
        SlimeBomb() : x(0), y(0), z(0), vx(0), vy(0), vz(0), targetX(0), targetZ(0),
                     life(0), maxLife(2.0f), size(0.3f), isActive(false) {}
    };
    std::vector<SlimeBomb> bombs;

    // Particle system for slime effects
    struct SlimeParticle {
        float x, y, z;
        float vx, vy, vz;
        float life;
        float maxLife;
        float size;
        float opacity;
        int type; // 0=trail, 1=shield, 2=bomb explosion
        SlimeParticle() : x(0), y(0), z(0), vx(0), vy(0), vz(0), life(0), maxLife(1),
                         size(0.1f), opacity(1.0f), type(0) {}
    };
    std::vector<SlimeParticle> particles;

    // Color structure
    struct Color {
        float r, g, b, a;
        Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
            : r(red), g(green), b(blue), a(alpha) {}
    };

    // Drawing functions
    void drawSlimeBody();
    void drawSlimeEyes();
    void drawSlimeMouth();
    void drawShield();
    void drawHealthBar();
    void drawParticles();
    void drawBombs();

    // Animation functions
    void updateIdleAnimation(float deltaTime);
    void updateWalkingAnimation(float deltaTime);
    void updateThrowingAnimation(float deltaTime);
    void updateShieldAnimation(float deltaTime);
    void updateHurtAnimation(float deltaTime);
    void updateBouncingAnimation(float deltaTime);

    // Particle and bomb systems
    void updateParticles(float deltaTime);
    void updateBombs(float deltaTime);
    void addSlimeTrailParticles();
    void addShieldParticles();
    void addBombExplosionParticles(float x, float y, float z);
    void createSlimeBomb(float targetX, float targetZ);

    // Utility functions
    void setMaterial(const Color& ambient, const Color& diffuse, const Color& specular, float shininess);
    void drawVoxelCube(float x, float y, float z, float size, const Color& color);
    float distanceTo(float tx, float tz);
    void moveTowardsTarget(float deltaTime);

public:
    Slime(float startX = 0.0f, float startZ = 0.0f);
    ~Slime();

    // Main functions
    void update(float deltaTime);
    void render();
    void reset();

    // Movement functions (step-based and continuous)
    void stepMove(float deltaX, float deltaZ); // One step per key press
    void startWalking(float deltaX, float deltaZ); // Continuous movement when held
    void stopWalking();

       void setRotation(float rotation) { rotationY = rotation; }
       float getRotation() const { return rotationY; }

    // Combat functions
    void takeDamage(float damage);
    void activateShield();
    bool canThrow();
    void throwSlimeBomb(float targetX, float targetZ);
    void bounce(); // Bounce effect for dodging

    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    bool getShieldStatus() const { return isShieldActive; }
    bool getIsMoving() const { return isMoving; }
    bool getIsThrowing() const { return isThrowing; }
    AnimationState getCurrentState() const { return currentState; }

    // Setters
    void setPosition(float newX, float newZ);
    void setTarget(float newTargetX, float newTargetZ);

    // AI behavior (for future use)
    void moveTowards(float targetX, float targetZ, float speed);
    bool isInRange(float targetX, float targetZ, float range);

    // Bomb collision detection (for damage dealing)
    bool checkBombCollisions(float targetX, float targetZ, float radius);
};

#endif // SLIME_HPP
