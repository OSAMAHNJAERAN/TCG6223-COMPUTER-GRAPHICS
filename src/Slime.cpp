#include "Slime.hpp"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>

Slime::Slime(float startX, float startZ) {
    // Initialize character stats
    maxHealth = 120.0f; // Slightly more health than Ghost
    health = maxHealth;
    isShieldActive = false;
    shieldDuration = 0.0f;
    shieldCooldown = 0.0f;

    // Initialize position
    x = startX;
    y = 1.2f; // Raise slime higher off the ground
    z = startZ;
    targetX = startX;
    targetZ = startZ;

    // Initialize movement
    isMoving = false;
    moveSpeed = 3.0f; // Slower than Ghost dash
    stepDistance = 1.0f; // Distance per step

    // Initialize animation
    currentState = IDLE;
    animationTime = 0.0f;
    stateTime = 0.0f;
    bounceOffset = 0.0f;
    squishFactor = 1.0f;
    glowIntensity = 1.0f;
    isFlashing = false;
    flashTime = 0.0f;

    // Initialize attack - BALANCED DAMAGE
    attackDamage = 12.0f; // Increased from 8.0f to 12.0f for better balance
    attackRange = 8.0f; // Increased range for better targeting
    attackCooldown = 0.0f;
    isThrowing = false;
    throwAnimationTime = 0.0f;

    // Initialize systems
    particles.resize(100);
    bombs.resize(10);


}

Slime::~Slime() {
    // Cleanup
}

void Slime::update(float deltaTime) {
    animationTime += deltaTime;
    stateTime += deltaTime;

    // Update cooldowns
    if (attackCooldown > 0.0f) attackCooldown -= deltaTime;
    if (shieldCooldown > 0.0f) shieldCooldown -= deltaTime;

    // Update shield
    if (isShieldActive) {
        shieldDuration -= deltaTime;
        if (shieldDuration <= 0.0f) {
            isShieldActive = false;
            shieldCooldown = 6.0f; // 6 second cooldown
        }
    }

    // Update flash effect
    if (isFlashing) {
        flashTime -= deltaTime;
        if (flashTime <= 0.0f) {
            isFlashing = false;
        }
    }

    // Update movement if walking
    if (isMoving && currentState == WALKING) {
        moveTowardsTarget(deltaTime);
    }

    // Update current animation state
    switch (currentState) {
        case IDLE:
            updateIdleAnimation(deltaTime);
            break;
        case WALKING:
            updateWalkingAnimation(deltaTime);
            break;
        case THROWING:
            updateThrowingAnimation(deltaTime);
            break;
        case SHIELDING:
            updateShieldAnimation(deltaTime);
            break;
        case HURT:
            updateHurtAnimation(deltaTime);
            break;
        case BOUNCING:
            updateBouncingAnimation(deltaTime);
            break;
    }

    // Update systems
    updateParticles(deltaTime);
    updateBombs(deltaTime);

    // Add continuous slime trail particles
    if (animationTime - floor(animationTime) < 0.15f) {
        addSlimeTrailParticles();
    }
}

void Slime::updateIdleAnimation(float deltaTime) {
    // Gentle breathing/pulsing animation
    bounceOffset = sin(animationTime * 1.5f) * 0.1f;

    // Slight squish effect for slime-like appearance
    squishFactor = 1.0f + sin(animationTime * 2.0f) * 0.05f;

    // Gentle glow pulsing
    glowIntensity = 0.9f + 0.2f * sin(animationTime * 1.2f);
}

void Slime::updateWalkingAnimation(float deltaTime) {
    // Bouncy walking animation
    bounceOffset = fabs(sin(animationTime * 8.0f)) * 0.3f;

    // Squish effect when bouncing
    float bouncePhase = sin(animationTime * 8.0f);
    squishFactor = 1.0f + bouncePhase * 0.15f;

    // Walking glow
    glowIntensity = 1.2f + sin(animationTime * 6.0f) * 0.3f;
}

void Slime::updateThrowingAnimation(float deltaTime) {
    throwAnimationTime += deltaTime;

    if (throwAnimationTime < 0.3f) {
        // Wind-up phase - compress
        squishFactor = 1.0f - throwAnimationTime * 0.5f;
        glowIntensity = 1.5f;
    } else if (throwAnimationTime < 0.6f) {
        // Throw phase - expand
        squishFactor = 0.85f + (throwAnimationTime - 0.3f) * 2.0f;
        glowIntensity = 2.0f;
        bounceOffset = 0.2f;
    } else {
        // Recovery phase
        isThrowing = false;
        currentState = IDLE;
        stateTime = 0.0f;
        throwAnimationTime = 0.0f;
        attackCooldown = 1.5f; // Reduced cooldown from 2.0f to 1.5f for faster attacks
        squishFactor = 1.0f;
        bounceOffset = 0.0f;
    }
}

void Slime::updateShieldAnimation(float deltaTime) {
    if (isShieldActive) {
        glowIntensity = 2.5f + sin(animationTime * 10.0f) * 0.5f;
        squishFactor = 1.1f; // Slightly bigger when shielded
        addShieldParticles();
    }

    // Return to idle after shield animation
    if (stateTime > 1.0f && !isShieldActive) {
        currentState = IDLE;
        stateTime = 0.0f;
    }
}

void Slime::updateHurtAnimation(float deltaTime) {
    // Flash red and squish when hurt
    isFlashing = true;
    flashTime = 0.3f;
    squishFactor = 0.8f;

    // Return to idle
    if (stateTime > 0.6f) {
        currentState = IDLE;
        stateTime = 0.0f;
        squishFactor = 1.0f;
    }
}

void Slime::updateBouncingAnimation(float deltaTime) {
    // Big bounce effect for dodging
    if (stateTime < 0.4f) {
        bounceOffset = sin(stateTime * 15.0f) * 1.0f; // High bounce
        squishFactor = 1.0f + sin(stateTime * 15.0f) * 0.3f;
        glowIntensity = 2.0f;
    } else {
        // Land and return to idle
        currentState = IDLE;
        stateTime = 0.0f;
        bounceOffset = 0.0f;
        squishFactor = 1.0f;
    }
}

void Slime::render() {
    glPushMatrix();
    // Apply position and bounce
    glTranslatef(x, y + bounceOffset, z);
    // Apply rotation to face the target
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    // Apply squish effect
    glScalef(squishFactor, 1.0f / squishFactor, squishFactor);
    // Apply flash effect
    if (isFlashing) {
        float flashIntensity = sin(flashTime * 60.0f);
        glColor4f(1.0f, 0.3f, 0.3f, 0.8f + 0.2f * flashIntensity);
    }
    // Enable blending for slime transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Draw slime components
    drawSlimeBody();
    drawSlimeEyes();
    drawSlimeMouth();
    // Draw shield if active
    if (isShieldActive) {
        drawShield();
    }
    glDisable(GL_BLEND);
    glPopMatrix();
    // Draw world-space elements
    drawParticles();
    drawBombs();
    // Draw health bar above slime
    glPushMatrix();
    glTranslatef(x, y + 2.5f, z);
    drawHealthBar();
    glPopMatrix();
}


void Slime::drawSlimeBody() {
    // Set slime material - bright green like minecraft slime but cubic like ghost
    Color slimeColor(0.2f, 0.8f, 0.2f, 0.9f); // Bright green, mostly solid
    setMaterial(Color(0.1f, 0.4f, 0.1f, 0.9f),
                slimeColor,
                Color(0.3f, 1.0f, 0.3f, 0.9f),
                20.0f);

    // Main body (cubic like ghost but green)
    glPushMatrix();
    glScalef(1.0f, 1.3f, 0.8f); // Similar proportions to ghost
    glColor4f(slimeColor.r, slimeColor.g, slimeColor.b, slimeColor.a);
    glutSolidCube(1.8f); // Cubic body like ghost
    glPopMatrix();

    // Head (cubic head like ghost)
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glColor4f(slimeColor.r * 1.1f, slimeColor.g * 1.1f, slimeColor.b * 1.1f, slimeColor.a);
    glutSolidCube(1.3f); // Cubic head like ghost
    glPopMatrix();
}

void Slime::drawSlimeEyes() {
    // Glowing GREEN eyes like ghost but green colored
    Color eyeColor(0.1f, 1.0f, 0.1f, 1.0f); // Bright green eyes
    setMaterial(eyeColor, eyeColor, Color(0.5f, 1.0f, 0.5f), 50.0f);

    // Left eye (cubic style like ghost)
    glPushMatrix();
    glTranslatef(-0.25f, 1.2f, 0.6f);
    glColor4f(eyeColor.r, eyeColor.g, eyeColor.b, eyeColor.a);
    glutSolidCube(0.2f); // Glowing green cubic eyes
    glPopMatrix();

    // Right eye (cubic style like ghost)
    glPushMatrix();
    glTranslatef(0.25f, 1.2f, 0.6f);
    glColor4f(eyeColor.r, eyeColor.g, eyeColor.b, eyeColor.a);
    glutSolidCube(0.2f); // Glowing green cubic eyes
    glPopMatrix();

    // Eye glow effect when throwing (like ghost)
    if (currentState == THROWING) {
        glDisable(GL_LIGHTING);
        Color glowColor(0.2f, 1.0f, 0.2f, 0.7f); // Bright green glow

        // Left eye glow
        glPushMatrix();
        glTranslatef(-0.25f, 1.2f, 0.6f);
        glColor4f(glowColor.r, glowColor.g, glowColor.b, glowColor.a);
        glutSolidCube(0.5f); // Larger glow cube
        glPopMatrix();

        // Right eye glow
        glPushMatrix();
        glTranslatef(0.25f, 1.2f, 0.6f);
        glColor4f(glowColor.r, glowColor.g, glowColor.b, glowColor.a);
        glutSolidCube(0.5f); // Larger glow cube
        glPopMatrix();

        glEnable(GL_LIGHTING);
    }
}

void Slime::drawSlimeMouth() {
    // Simple mouth - small dark green cube (like ghost but green themed)
    Color mouthColor(0.0f, 0.4f, 0.0f, 1.0f); // Dark green
    setMaterial(mouthColor, mouthColor, Color(0.1f, 0.2f, 0.1f), 5.0f);

    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.6f); // Positioned like ghost mouth
    glColor4f(mouthColor.r, mouthColor.g, mouthColor.b, mouthColor.a);
    glutSolidCube(0.12f);
    glPopMatrix();
}

void Slime::drawShield() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Green shield with hexagonal pattern
    float shieldPulse = 1.0f + 0.4f * sin(animationTime * 12.0f);
    Color shieldColor(0.2f, 1.0f, 0.3f, 0.4f); // Bright green

    // Outer shield
    glPushMatrix();
    glScalef(shieldPulse, shieldPulse, shieldPulse);
    glColor4f(shieldColor.r, shieldColor.g, shieldColor.b, shieldColor.a);
    glutWireSphere(2.2f, 16, 12);
    glPopMatrix();

    // Inner glow
    glPushMatrix();
    glScalef(shieldPulse * 0.7f, shieldPulse * 0.7f, shieldPulse * 0.7f);
    glColor4f(shieldColor.r, shieldColor.g, shieldColor.b, shieldColor.a * 0.3f);
    glutSolidSphere(2.2f, 12, 8);
    glPopMatrix();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}

void Slime::drawHealthBar() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    float barWidth = 2.2f;
    float barHeight = 0.25f;
    float healthPercent = health / maxHealth;

    // Background
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-barWidth/2, 0, 0);
    glVertex3f(barWidth/2, 0, 0);
    glVertex3f(barWidth/2, barHeight, 0);
    glVertex3f(-barWidth/2, barHeight, 0);
    glEnd();

    // Health bar (green theme)
    if (healthPercent > 0.6f) {
        glColor3f(0.1f, 0.9f, 0.1f); // Bright green
    } else if (healthPercent > 0.3f) {
        glColor3f(0.7f, 0.9f, 0.1f); // Yellow-green
    } else {
        glColor3f(0.9f, 0.3f, 0.1f); // Red
    }

    glBegin(GL_QUADS);
    glVertex3f(-barWidth/2, 0, 0);
    glVertex3f(-barWidth/2 + barWidth * healthPercent, 0, 0);
    glVertex3f(-barWidth/2 + barWidth * healthPercent, barHeight, 0);
    glVertex3f(-barWidth/2, barHeight, 0);
    glEnd();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void Slime::updateParticles(float deltaTime) {
    for (std::vector<SlimeParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
        SlimeParticle& p = *it;
        if (p.life > 0.0f) {
            p.x += p.vx * deltaTime;
            p.y += p.vy * deltaTime;
            p.z += p.vz * deltaTime;
            p.vy -= 5.0f * deltaTime; // Gravity
            p.life -= deltaTime;
            p.opacity = p.life / p.maxLife;
        }
    }
}

void Slime::updateBombs(float deltaTime) {
    for (std::vector<SlimeBomb>::iterator it = bombs.begin(); it != bombs.end(); ++it) {
        SlimeBomb& bomb = *it;
        if (bomb.isActive) {
            // Update bomb physics
            bomb.x += bomb.vx * deltaTime;
            bomb.y += bomb.vy * deltaTime;
            bomb.z += bomb.vz * deltaTime;
            bomb.vy -= 9.8f * deltaTime; // Gravity

            bomb.life -= deltaTime;

            // Check if bomb hits ground or expires
            if (bomb.y <= 0.2f || bomb.life <= 0.0f) {
                // Explode!
                addBombExplosionParticles(bomb.x, bomb.y, bomb.z);
                bomb.isActive = false;
                std::cout << "Slime bomb exploded at (" << bomb.x << ", " << bomb.z << ")!" << std::endl;
            }
        }
    }
}

void Slime::addSlimeTrailParticles() {
    for (int i = 0; i < 2; i++) {
        for (std::vector<SlimeParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            SlimeParticle& p = *it;
            if (p.life <= 0.0f) {
                p.x = x + ((rand() % 100) - 50) * 0.02f;
                p.y = y + bounceOffset + ((rand() % 50)) * 0.01f;
                p.z = z + ((rand() % 100) - 50) * 0.02f;
                p.vx = ((rand() % 100) - 50) * 0.003f;
                p.vy = ((rand() % 50) + 10) * 0.01f;
                p.vz = ((rand() % 100) - 50) * 0.003f;
                p.life = 0.8f + (rand() % 50) * 0.01f;
                p.maxLife = p.life;
                p.size = 0.03f + (rand() % 10) * 0.003f;
                p.opacity = 0.7f;
                p.type = 0; // Trail particle
                break;
            }
        }
    }
}

void Slime::addShieldParticles() {
    for (int i = 0; i < 3; i++) {
        for (std::vector<SlimeParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            SlimeParticle& p = *it;
            if (p.life <= 0.0f) {
                float angle = (rand() % 360) * 3.14159f / 180.0f;
                float radius = 2.2f + (rand() % 30) * 0.02f;

                p.x = x + cos(angle) * radius;
                p.y = y + bounceOffset + ((rand() % 100) - 50) * 0.02f;
                p.z = z + sin(angle) * radius;
                p.vx = cos(angle) * 0.3f;
                p.vy = ((rand() % 50)) * 0.01f;
                p.vz = sin(angle) * 0.3f;
                p.life = 0.6f;
                p.maxLife = p.life;
                p.size = 0.08f;
                p.opacity = 0.9f;
                p.type = 1; // Shield particle
                break;
            }
        }
    }
}

void Slime::addBombExplosionParticles(float x, float y, float z) {
    for (int i = 0; i < 15; i++) {
        for (std::vector<SlimeParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            SlimeParticle& p = *it;
            if (p.life <= 0.0f) {
                float angle = (rand() % 360) * 3.14159f / 180.0f;
                float speed = 1.0f + (rand() % 100) * 0.05f;

                p.x = x;
                p.y = y;
                p.z = z;
                p.vx = cos(angle) * speed;
                p.vy = ((rand() % 100) + 50) * 0.02f;
                p.vz = sin(angle) * speed;
                p.life = 1.5f;
                p.maxLife = p.life;
                p.size = 0.12f + (rand() % 20) * 0.005f;
                p.opacity = 1.0f;
                p.type = 2; // Explosion particle
                break;
            }
        }
    }
}

void Slime::drawParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (std::vector<SlimeParticle>::const_iterator it = particles.begin(); it != particles.end(); ++it) {
        const SlimeParticle& p = *it;
        if (p.life > 0.0f) {
            // Different colors based on particle type
            switch (p.type) {
                case 0: // Trail particles
                    glColor4f(0.3f, 0.9f, 0.3f, p.opacity * 0.6f);
                    break;
                case 1: // Shield particles
                    glColor4f(0.2f, 1.0f, 0.4f, p.opacity * 0.8f);
                    break;
                case 2: // Explosion particles
                    glColor4f(0.9f, 0.7f, 0.2f, p.opacity);
                    break;
            }

            glPushMatrix();
            glTranslatef(p.x, p.y, p.z);
            glutSolidSphere(p.size, 4, 3);
            glPopMatrix();
        }
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}

void Slime::drawBombs() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (std::vector<SlimeBomb>::const_iterator it = bombs.begin(); it != bombs.end(); ++it) {
        const SlimeBomb& bomb = *it;
        if (bomb.isActive) {
            // Draw spinning slime bomb
            Color bombColor(0.8f, 0.9f, 0.2f, 0.9f); // Yellow-green

            glPushMatrix();
            glTranslatef(bomb.x, bomb.y, bomb.z);
            glRotatef(bomb.life * 360.0f, 1, 1, 0); // Spin effect
            glColor4f(bombColor.r, bombColor.g, bombColor.b, bombColor.a);
            glutSolidSphere(bomb.size, 8, 6);

            // Glowing core
            glColor4f(1.0f, 1.0f, 0.5f, 0.5f);
            glutSolidSphere(bomb.size * 0.6f, 6, 4);

            glPopMatrix();
        }
    }

    glDisable(GL_BLEND);
}

void Slime::setMaterial(const Color& ambient, const Color& diffuse, const Color& specular, float shininess) {
    GLfloat mat_ambient[] = {ambient.r, ambient.g, ambient.b, ambient.a};
    GLfloat mat_diffuse[] = {diffuse.r, diffuse.g, diffuse.b, diffuse.a};
    GLfloat mat_specular[] = {specular.r, specular.g, specular.b, specular.a};

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

// Movement functions
void Slime::stepMove(float deltaX, float deltaZ) {
    if (currentState == IDLE || currentState == WALKING) {
        // One step movement
        x += deltaX * stepDistance;
        z += deltaZ * stepDistance;

        // Brief bounce animation
        currentState = BOUNCING;
        stateTime = 0.0f;

    }
}

void Slime::startWalking(float deltaX, float deltaZ) {
    if (currentState == IDLE || currentState == WALKING) {
        // Set target for continuous movement
        targetX = x + deltaX * 5.0f; // Walk 5 units in direction
        targetZ = z + deltaZ * 5.0f;

        isMoving = true;
        currentState = WALKING;
        stateTime = 0.0f;


    }
}

void Slime::stopWalking() {
    if (currentState == WALKING) {
        isMoving = false;
        currentState = IDLE;
        stateTime = 0.0f;


    }
}

void Slime::moveTowardsTarget(float deltaTime) {
    if (!isMoving) return;

    float dx = targetX - x;
    float dz = targetZ - z;
    float distance = sqrt(dx * dx + dz * dz);

    if (distance < 0.3f) {
        // Reached target
        stopWalking();
        return;
    }

    // Move towards target
    float moveDistance = moveSpeed * deltaTime;
    if (moveDistance > distance) {
        moveDistance = distance;
    }

    x += (dx / distance) * moveDistance;
    z += (dz / distance) * moveDistance;
}

// Combat functions
void Slime::takeDamage(float damage) {
    if (isShieldActive) {

        return;
    }

    health -= damage;
    if (health < 0.0f) health = 0.0f;

    currentState = HURT;
    stateTime = 0.0f;


}

void Slime::activateShield() {
    if (shieldCooldown <= 0.0f && !isShieldActive) {
        isShieldActive = true;
        shieldDuration = 4.0f; // Shield lasts 4 seconds
        currentState = SHIELDING;
        stateTime = 0.0f;


    }
}

bool Slime::canThrow() {
    return attackCooldown <= 0.0f && !isThrowing && currentState != BOUNCING;
}

void Slime::throwSlimeBomb(float targetX, float targetZ) {
    if (canThrow()) {
        isThrowing = true;
        currentState = THROWING;
        stateTime = 0.0f;
        throwAnimationTime = 0.0f;

        // Create the bomb projectile with IMPROVED targeting
        createSlimeBomb(targetX, targetZ);


    }
}

// IMPROVED BOMB TARGETING SYSTEM
void Slime::createSlimeBomb(float targetX, float targetZ) {
    // Find an inactive bomb slot
    for (std::vector<SlimeBomb>::iterator it = bombs.begin(); it != bombs.end(); ++it) {
        SlimeBomb& bomb = *it;
        if (!bomb.isActive) {
            bomb.isActive = true;
            bomb.x = x;
            bomb.y = y + 1.0f; // Start above slime
            bomb.z = z;
            bomb.targetX = targetX;
            bomb.targetZ = targetZ;
            bomb.life = bomb.maxLife;

            // IMPROVED PREDICTIVE TARGETING
            float dx = targetX - x;
            float dz = targetZ - z;
            float horizontalDistance = sqrt(dx * dx + dz * dz);

            if (horizontalDistance > 0.1f) {
                // Enhanced ballistic trajectory calculation
                float gravity = 9.8f;
                float launchAngle = 45.0f * 3.14159f / 180.0f; // 45 degree angle for optimal range

                // Calculate initial velocity needed for this distance
                float initialVelocity = sqrt((horizontalDistance * gravity) / sin(2.0f * launchAngle));

                // Cap the velocity to reasonable limits
                if (initialVelocity > 15.0f) initialVelocity = 15.0f;
                if (initialVelocity < 8.0f) initialVelocity = 8.0f;

                // Calculate velocity components
                bomb.vx = (dx / horizontalDistance) * initialVelocity * cos(launchAngle);
                bomb.vz = (dz / horizontalDistance) * initialVelocity * cos(launchAngle);
                bomb.vy = initialVelocity * sin(launchAngle);


            } else {
                // Target is very close - drop bomb
                bomb.vx = 0.0f;
                bomb.vz = 0.0f;
                bomb.vy = 3.0f;
            }

            break;
        }
    }
}

void Slime::bounce() {
    if (currentState == IDLE || currentState == WALKING) {
        currentState = BOUNCING;
        stateTime = 0.0f;


    }
}

bool Slime::checkBombCollisions(float targetX, float targetZ, float radius) {
    bool hit = false;
    for (std::vector<SlimeBomb>::iterator it = bombs.begin(); it != bombs.end();) {
        SlimeBomb& bomb = *it;

        // Only check active bombs
        if (bomb.isActive) {
            float dx = bomb.x - targetX;
            float dz = bomb.z - targetZ;
            float distance = sqrt(dx * dx + dz * dz);

            // Use a much smaller collision radius for direct hit detection
            // This should be roughly the size of the ghost model
            float hitRadius = 1.0f; // Reduced from the passed radius parameter

            // Check collision only when bomb is very close to ghost
            if (distance <= hitRadius) {
                // Hit! Explode the bomb at the collision point
                addBombExplosionParticles(bomb.x, bomb.y, bomb.z);

                // Remove the bomb from the vector immediately
                it = bombs.erase(it);
                hit = true;

                // Don't increment iterator since we erased an element
                continue;
            }

            // Check if bomb has landed on ground near ghost (for area damage)
            else if (bomb.y <= 0.1f && distance <= 1.5f) {
                // Ground explosion with small area effect
                addBombExplosionParticles(bomb.x, 0.5f, bomb.z);
                it = bombs.erase(it);
                hit = true;
                continue;
            }
        }

        // Only increment iterator if we didn't erase an element
        ++it;
    }
    return hit;
}

// Utility functions
float Slime::distanceTo(float tx, float tz) {
    float dx = tx - x;
    float dz = tz - z;
    return sqrt(dx * dx + dz * dz);
}

void Slime::setPosition(float newX, float newZ) {
    x = newX;
    z = newZ;
}

void Slime::setTarget(float newTargetX, float newTargetZ) {
    targetX = newTargetX;
    targetZ = newTargetZ;
}

void Slime::moveTowards(float targetX, float targetZ, float speed) {
    if (currentState == IDLE) {
        float dx = targetX - x;
        float dz = targetZ - z;
        float distance = sqrt(dx * dx + dz * dz);

        if (distance > 0.1f) {
            float moveDistance = speed * 0.016f; // Assuming 60 FPS
            x += (dx / distance) * moveDistance;
            z += (dz / distance) * moveDistance;
        }
    }
}

bool Slime::isInRange(float targetX, float targetZ, float range) {
    return distanceTo(targetX, targetZ) <= range;
}

void Slime::reset() {
    health = maxHealth;
    isShieldActive = false;
    shieldDuration = 0.0f;
    shieldCooldown = 0.0f;
    isMoving = false;
    attackCooldown = 0.0f;
    isThrowing = false;
    currentState = IDLE;
    animationTime = 0.0f;
    stateTime = 0.0f;
    isFlashing = false;
    flashTime = 0.0f;
    bounceOffset = 0.0f;
    squishFactor = 1.0f;

    // Deactivate all bombs
    for (std::vector<SlimeBomb>::iterator it = bombs.begin(); it != bombs.end(); ++it) {
        it->isActive = false;
    }
}
