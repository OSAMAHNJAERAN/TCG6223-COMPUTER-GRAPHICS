#ifndef GHOST_HPP
#define GHOST_HPP

#include <GL/glut.h>
#include <vector>
#include <cmath>

class Ghost {
private:

    float rotationY = 0.0f;
    // Character stats
    float health;
    float maxHealth;
    bool isShieldActive;
    float shieldDuration;
    float shieldCooldown;
    float leftArmAngle, rightArmAngle;
    float leftHandAngle, rightHandAngle;
    float bodySway;

    // Position and movement
    float x, y, z;
    float targetX, targetZ;
    bool isDashing;
    float dashSpeed;
    float dashCooldown;
    float dashDuration;

    // Attack properties
    float swordAttackDamage;
    float regularAttackDamage;
    float attackRange;
    float attackDamage;
    float attackCooldown;
    bool isAttacking;
    float attackAnimationTime;
    bool usingSword;

    // Hand and sword animation properties
    float leftHandAngleX, leftHandAngleY, leftHandAngleZ;
    float rightHandAngleX, rightHandAngleY, rightHandAngleZ;
    float leftHandExtension, rightHandExtension;
    float swordAngleX, swordAngleY, swordAngleZ;
    float rightArmAngleX ;
    float rightArmAngleY ;
    float leftArmAngleX ;
    float leftArmAngleY ;
    bool handsAnimating;
    float handAnimationTime;
    float handSwipePhase;

    // Animation states
    enum AnimationState {
        IDLE,
        ATTACKING,
        DASHING,
        SHIELDING,
        HURT,
        INVISIBLE // New invisibility state
    };
    AnimationState currentState;
    float animationTime;
    float stateTime;

    // Visual effects
    float floatOffset;
    float ghostOpacity;
    float glowIntensity;
    bool isFlashing;
    float flashTime;
    bool isInvisible;
    float invisibilityDuration;
    float invisibilityCooldown;



    // Particle system for ghost effects
    struct GhostParticle {
        float x, y, z;
        float vx, vy, vz;
        float life;
        float maxLife;
        float size;
        float opacity;
        int type; // 0=trail, 1=shield, 2=attack, 3=sword, 4=walk, 5=teleport
        GhostParticle() : x(0), y(0), z(0), vx(0), vy(0), vz(0), life(0), maxLife(1), size(0.1f), opacity(1.0f), type(0) {}
    };
    std::vector<GhostParticle> particles;

    // Color structure
    struct Color {
        float r, g, b, a;
        Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
            : r(red), g(green), b(blue), a(alpha) {}
    };
    bool hasSword;
    float swordAngle;
    float swordSwingProgress;
    void equipSword();
    void unequipSword();

    // Drawing functions
    void  drawGhostHands();
    void drawGhostArms();
    void drawGhostBody();
    void drawGhostEyes();
    void drawGhostTail();
    void drawShield();
    void drawShadowSwipeEffect();
    void drawHealthBar();
    void drawParticles();
    void drawGhostSword();
    void drawSwordAttackEffect();

    // Animation functions
    void updateIdleAnimation(float deltaTime);
    void updateDashAnimation(float deltaTime);
    void updateAttackAnimation(float deltaTime);
    void updateShieldAnimation(float deltaTime);
    void updateHurtAnimation(float deltaTime);
    void updateInvisibilityAnimation(float deltaTime);


    // Particle system
    void updateParticles(float deltaTime);
    void addGhostTrailParticles();
    void addShieldParticles();
    void addAttackParticles();
    void addDashParticles();
    void addSwordParticles();
    void addSwordDashParticles();

    // Utility functions
    void setMaterial(const Color& ambient, const Color& diffuse, const Color& specular, float shininess);
    void drawVoxelCube(float x, float y, float z, float size, const Color& color);
    void drawGlowingSphere(float x, float y, float z, float radius, const Color& color, float glow);
    float distanceTo(float tx, float tz);

public:
    Ghost(float startX = 0.0f, float startZ = 0.0f);
    ~Ghost();

    // Main functions
    void update(float deltaTime);
    void render();
    void reset();

    // Combat functions
    void takeDamage(float damage);
    void activateShield();
    bool canDash();
    void dash(float targetX, float targetZ);
    bool canAttack();
    void shadowSwipe(float targetX, float targetZ);
    bool canGoInvisible();
    void goInvisible(); // New invisibility skill

    void setRotation(float rotation) { rotationY = rotation; }
    float getRotation() const { return rotationY; }

    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    bool getShieldStatus() const { return isShieldActive; }
    bool getIsDashing() const { return isDashing; }
    bool getIsAttacking() const { return isAttacking; }
    bool getIsInvisible() const { return isInvisible; }
    AnimationState getCurrentState() const { return currentState; }

    // Setters
    void setPosition(float newX, float newZ);
    void setTarget(float newTargetX, float newTargetZ);
    void setSword(bool equipped);

    // AI behavior (for future use)
    void moveTowards(float targetX, float targetZ, float speed);
    bool isInRange(float targetX, float targetZ, float range);
};

#endif // GHOST_HPP
