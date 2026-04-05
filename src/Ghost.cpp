#include "Ghost.hpp"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>

Ghost::Ghost(float startX, float startZ) {
    // Initialize character stats
    maxHealth = 100.0f;
    health = maxHealth;
    isShieldActive = false;
    shieldDuration = 0.0f;
    shieldCooldown = 0.0f;

    // Initialize sword variables
    hasSword = true;  // ENABLE SWORD BY DEFAULT FOR TESTING
    swordAngleX = 0.0f;
    swordAngleY = 0.0f;
    swordAngleZ = 0.0f;
    swordSwingProgress = 0.0f;

    // Set sword damage
    if (hasSword) {
        attackDamage = 15.0f; // Sword damage
        attackRange = 3.5f;   // Longer reach
    } else {
        attackDamage = 10.0f; // Default damage
        attackRange = 3.0f;   // Default range
    }
       // Initialize hand animation
    leftHandAngleX = 0.0f; leftHandAngleY = 0.0f; leftHandAngleZ = 0.0f;
    rightHandAngleX = 0.0f; rightHandAngleY = 0.0f; rightHandAngleZ = 0.0f;
    leftHandExtension = 0.0f; rightHandExtension = 0.0f;
    handsAnimating = false;
    handAnimationTime = 0.0f;
    handSwipePhase = 0.0f;

    rightArmAngleX = 0.0f;
    rightArmAngleY = 0.0f;
    leftArmAngleX = 0.0f;
    leftArmAngleY = 0.0f;

    // Initialize position
    x = startX;
    y = 2.0f; // Ghost floats above ground
    z = startZ;
    targetX = startX;
    targetZ = startZ;

    // Initialize movement
    isDashing = false;
    dashSpeed = 15.0f;
    dashCooldown = 0.0f;
    dashDuration = 0.0f;

    // Initialize animation
    currentState = IDLE;
    animationTime = 0.0f;
    stateTime = 0.0f;
    floatOffset = 0.0f;
    ghostOpacity = 0.95f; // Start mostly solid
    glowIntensity = 1.0f;
    isFlashing = false;
    flashTime = 0.0f;
    isInvisible = false;
    invisibilityDuration = 0.0f;
    invisibilityCooldown = 0.0f;

        // Initialize attack - ENHANCED
    swordAttackDamage = 18.0f;     // NEW: Power sword damage
    regularAttackDamage = 10.0f;   // NEW: Regular attack damage
    attackRange = 3.0f;
    attackCooldown = 0.0f;
    isAttacking = false;
    attackAnimationTime = 0.0f;
    usingSword = false;            // NEW: Sword flag

    // Initialize attack - BALANCED DAMAGE

    attackRange = 3.0f;
    attackCooldown = 0.0f;
    isAttacking = false;
    attackAnimationTime = 0.0f;


    swordAngle = 0.0f;
    swordSwingProgress = 0.0f;

    // Initialize particle system
    particles.resize(50);


}

Ghost::~Ghost() {
    // Cleanup
}

void Ghost::update(float deltaTime) {
    animationTime += deltaTime;
    stateTime += deltaTime;

    // Update cooldowns
    if (dashCooldown > 0.0f) dashCooldown -= deltaTime;
    if (attackCooldown > 0.0f) attackCooldown -= deltaTime;
    if (shieldCooldown > 0.0f) shieldCooldown -= deltaTime;
    if (invisibilityCooldown > 0.0f) invisibilityCooldown -= deltaTime;

    // Update shield
    if (isShieldActive) {
        shieldDuration -= deltaTime;
        if (shieldDuration <= 0.0f) {
            isShieldActive = false;
            shieldCooldown = 5.0f; // 5 second cooldown
        }
    }

    // Update invisibility
    if (isInvisible) {
    invisibilityDuration -= deltaTime;  // This should decrease every frame
    if (invisibilityDuration <= 0.0f) {
        isInvisible = false;
        invisibilityCooldown = 8.0f; // 8 second cooldown
         ghostOpacity = 0.95f;
        if (currentState == INVISIBLE) {
            currentState = IDLE;
            stateTime = 0.0f;
        }
    }
}

    // Update flash effect
    if (isFlashing) {
        flashTime -= deltaTime;
        if (flashTime <= 0.0f) {
            isFlashing = false;
        }
    }

    // Update current animation state
    switch (currentState) {
        case IDLE:
            updateIdleAnimation(deltaTime);
            break;
        case DASHING:
            updateDashAnimation(deltaTime);
            break;
        case ATTACKING:
            updateAttackAnimation(deltaTime);
            break;
        case SHIELDING:
            updateShieldAnimation(deltaTime);
            break;
        case HURT:
            updateHurtAnimation(deltaTime);
            break;
        case INVISIBLE:
            updateInvisibilityAnimation(deltaTime);
            break;
    }

    // Update particle system
    updateParticles(deltaTime);

    // Add continuous ghost trail particles
    if (animationTime - floor(animationTime) < 0.1f) {
        addGhostTrailParticles();
    }
}

void Ghost::updateIdleAnimation(float deltaTime) {

    // Gentle floating animation with easing
    float targetOffset = sin(animationTime * 2.0f) * 0.3f;
    floatOffset += (targetOffset - floatOffset) * 5.0f * deltaTime;

    // Gentle glow pulsing with easing
    float targetGlow = 0.8f + 0.3f * sin(animationTime * 1.5f);
    glowIntensity += (targetGlow - glowIntensity) * 3.0f * deltaTime;

    // Body sway animation
    bodySway = sin(animationTime * 1.2f) * 5.0f;

    // Arm sway animation - smooth alternating movement
    leftArmAngle = 15.0f + sin(animationTime * 1.5f) * 10.0f;
    rightArmAngle = 15.0f + sin(animationTime * 1.5f + 3.14159f) * 10.0f;

    // Hand subtle movements
    leftHandAngle = sin(animationTime * 2.0f) * 5.0f;
    rightHandAngle = sin(animationTime * 2.0f + 1.57f) * 5.0f;
}



void Ghost::updateDashAnimation(float deltaTime) {
    if (isDashing) {
        dashDuration -= deltaTime;

        // Different arm animations based on whether ghost has sword
        if (hasSword) {
            // SWORD STABBING ANIMATION - Point sword forward
            rightArmAngleX = -45.0f;  // Bring arm forward
            rightArmAngleY = 0.0f;    // Keep arm straight
            leftArmAngleX = 30.0f;    // Left arm back for balance
            leftArmAngleY = -20.0f;   // Slightly angled

            // Position sword for forward stab
            rightHandAngleX = -30.0f;  // Angle hand down slightly
            rightHandAngleY = 0.0f;    // Keep hand straight
            rightHandAngleZ = 0.0f;    // No twist

            // Sword angles for stabbing forward
            swordAngleX = -90.0f;      // Point sword forward/down
            swordAngleY = 0.0f;        // No side angle
            swordAngleZ = 0.0f;        // No twist

            // Left hand position for balance
            leftHandAngleX = 0.0f;
            leftHandAngleY = -30.0f;
            leftHandAngleZ = 0.0f;

        } else {
            // Original dash animation for no sword
            leftArmAngle = 45.0f + sin(animationTime * 20.0f) * 15.0f;
            rightArmAngle = 45.0f + sin(animationTime * 20.0f + 3.14159f) * 15.0f;

            // Hand movement
            leftHandAngle = sin(animationTime * 30.0f) * 20.0f;
            rightHandAngle = sin(animationTime * 30.0f + 3.14159f) * 20.0f;
        }

        // Move towards target
        float dx = targetX - x;
        float dz = targetZ - z;
        float distance = sqrt(dx * dx + dz * dz);

        // Fixed threshold - if very close to target OR time runs out, stop dashing
        if (distance < 0.5f || dashDuration <= 0.0f) {
            // Dash completed - snap to target position to avoid shaking
            x = targetX;
            z = targetZ;
            isDashing = false;
            dashCooldown = 3.0f; // 3 second cooldown
            currentState = IDLE;
            stateTime = 0.0f;
            ghostOpacity = 0.95f; // Reset to solid

            // Reset sword angles when dash ends
            if (hasSword) {
                swordAngleX = 0.0f;
                swordAngleY = 0.0f;
                swordAngleZ = 0.0f;
                rightHandAngleX = 0.0f;
                rightHandAngleY = 0.0f;
                rightHandAngleZ = 0.0f;
                leftHandAngleX = 0.0f;
                leftHandAngleY = 0.0f;
                leftHandAngleZ = 0.0f;
            }

        } else {
            // Continue dashing
            float moveDistance = dashSpeed * deltaTime;

            // Make sure we don't overshoot the target
            if (moveDistance > distance) {
                moveDistance = distance;
            }

            x += (dx / distance) * moveDistance;
            z += (dz / distance) * moveDistance;

            // Add dash particles
            addDashParticles();

            // Dash visual effects
            ghostOpacity = 0.3f + 0.4f * sin(animationTime * 20.0f);
            glowIntensity = 2.0f;

            // Add sword trail particles if has sword
            if (hasSword) {
                addSwordDashParticles();
            }
        }
    }
}
void Ghost::addSwordParticles() {                       // NEW: Sword particles
    for (int i = 0; i < 10; i++) {
        for (std::vector<GhostParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            GhostParticle& p = *it;
            if (p.life <= 0.0f) {
                // Particles along sword path
                float angle = (rand() % 360) * 3.14159f / 180.0f;
                float radius = (rand() % 100) * 0.02f;

                p.x = x + 1.2f + cos(angle) * radius;
                p.y = y + floatOffset - 1.0f;
                p.z = z + sin(angle) * radius;
                p.vx = cos(angle) * 12.0f;
                p.vy = ((rand() % 100) - 50) * 0.03f;
                p.vz = sin(angle) * 12.0f;
                p.life = 0.6f;
                p.maxLife = p.life;
                p.size = 0.2f;
                p.opacity = 1.0f;
                p.type = 3; // Sword particles
                break;
            }
        }
    }
}

void Ghost::addSwordDashParticles() {
    for (int i = 0; i < 8; i++) {  // More particles for dramatic effect
        for (std::vector<GhostParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            GhostParticle& p = *it;
            if (p.life <= 0.0f) {
                // Calculate sword tip position for particles
                float swordTipX = x + 1.5f;  // Approximate sword reach forward
                float swordTipY = y + floatOffset - 0.5f;  // Slightly below center
                float swordTipZ = z;

                // Add some randomness around sword tip
                float angle = (rand() % 360) * 3.14159f / 180.0f;
                float radius = (rand() % 30) * 0.02f;  // Small radius around sword tip

                p.x = swordTipX + cos(angle) * radius;
                p.y = swordTipY + ((rand() % 40) - 20) * 0.01f;
                p.z = swordTipZ + sin(angle) * radius;

                // Particles trail behind the sword
                p.vx = ((rand() % 100) - 80) * 0.02f;  // Mostly backward
                p.vy = ((rand() % 60) - 30) * 0.01f;   // Some vertical spread
                p.vz = ((rand() % 100) - 50) * 0.01f;  // Side spread

                p.life = 0.8f;
                p.maxLife = p.life;
                p.size = 0.08f + (rand() % 20) * 0.003f;
                p.opacity = 1.0f;
                p.type = 4; // Sword dash particles
                break;
            }
        }
    }
}
void Ghost::updateAttackAnimation(float deltaTime) {
    attackAnimationTime += deltaTime;
    handsAnimating = true;
    handAnimationTime += deltaTime;

    if (attackAnimationTime < 0.3f) {
        // Wind-up phase
        handSwipePhase = attackAnimationTime / 0.3f;

        if (hasSword) {
            // Sword wind-up - raise sword high
            rightHandAngleX = -90.0f * handSwipePhase;
            rightHandAngleY = 45.0f * handSwipePhase;
            leftHandAngleX = -45.0f * handSwipePhase;
            leftHandAngleY = -30.0f * handSwipePhase;
            swordAngleX = -45.0f * handSwipePhase;
        } else {
            // Regular attack wind-up
            leftHandAngleX = -60.0f * handSwipePhase;
            rightHandAngleX = -60.0f * handSwipePhase;
            leftHandAngleY = -45.0f * handSwipePhase;
            rightHandAngleY = 45.0f * handSwipePhase;
        }

        glowIntensity = 1.5f + sin(attackAnimationTime * 30.0f) * 0.5f;
        ghostOpacity = 0.8f;

    } else if (attackAnimationTime < 0.6f) {
        // Attack phase
        float swipeProgress = (attackAnimationTime - 0.3f) / 0.3f;
        swordSwingProgress = swipeProgress; // Update swing progress

        if (hasSword) { // Use hasSword instead of usingSword
            // Dramatic sword swing
            rightHandAngleX = -90.0f + 180.0f * swipeProgress;
            rightHandAngleY = 45.0f - 90.0f * swipeProgress;
            leftHandAngleX = -45.0f + 90.0f * swipeProgress;
            leftHandAngleY = -30.0f + 60.0f * swipeProgress;
            swordAngleX = -45.0f + 135.0f * swipeProgress;
            swordAngleZ = sin(swipeProgress * 3.14159f) * 45.0f;
        } else {
            // Regular shadow swipe
            leftHandAngleX = -60.0f + 120.0f * swipeProgress;
            rightHandAngleX = -60.0f + 120.0f * swipeProgress;
            leftHandAngleY = -45.0f + 90.0f * swipeProgress;
            rightHandAngleY = 45.0f - 90.0f * swipeProgress;
            leftHandAngleZ = sin(swipeProgress * 6.0f * 3.14159f) * 30.0f;
            rightHandAngleZ = -sin(swipeProgress * 6.0f * 3.14159f) * 30.0f;
        }

        glowIntensity = 3.0f;
        ghostOpacity = 1.0f;

        // Add particles during swing
        if (attackAnimationTime > 0.4f && attackAnimationTime < 0.55f) {
            if (hasSword) {
                addSwordParticles();
            } else {
                addAttackParticles();
            }
        }

    } else if (attackAnimationTime < 1.0f) {
        // Recovery phase
        float recoveryProgress = (attackAnimationTime - 0.6f) / 0.4f;

        if (hasSword) {
            rightHandAngleX = 90.0f * (1.0f - recoveryProgress);
            rightHandAngleY = -45.0f * (1.0f - recoveryProgress);
            leftHandAngleX = 45.0f * (1.0f - recoveryProgress);
            leftHandAngleY = 30.0f * (1.0f - recoveryProgress);
            swordAngleX = 90.0f * (1.0f - recoveryProgress);
            swordAngleZ = swordAngleZ * (1.0f - recoveryProgress);
        } else {
            leftHandAngleX = 60.0f * (1.0f - recoveryProgress);
            rightHandAngleX = 60.0f * (1.0f - recoveryProgress);
            leftHandAngleY = 45.0f * (1.0f - recoveryProgress);
            rightHandAngleY = -45.0f * (1.0f - recoveryProgress);
            leftHandAngleZ = leftHandAngleZ * (1.0f - recoveryProgress);
            rightHandAngleZ = rightHandAngleZ * (1.0f - recoveryProgress);
        }

        glowIntensity = 3.0f * (1.0f - recoveryProgress) + 1.0f;

    } else {
        // Attack finished
        isAttacking = false;
        handsAnimating = false;
        currentState = IDLE;
        stateTime = 0.0f;
        attackAnimationTime = 0.0f;
        attackCooldown = hasSword ? 3.5f : 2.5f;
        handAnimationTime = 0.0f;

        // Reset all angles
        leftHandAngleX = rightHandAngleX = 0.0f;
        leftHandAngleY = rightHandAngleY = 0.0f;
        leftHandAngleZ = rightHandAngleZ = 0.0f;
        leftHandExtension = rightHandExtension = 0.0f;
        swordAngleX = swordAngleY = swordAngleZ = 0.0f;
        swordSwingProgress = 0.0f;
    }
}
void Ghost::equipSword() {
    setSword(true);
}
void Ghost::unequipSword() {
    setSword(false);
}
void Ghost::drawSwordAttackEffect() {
    if (!hasSword) return;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Bright energy slash effect
    for (int i = 0; i < 4; i++) {
        float slashLength = 2.0f + i * 0.3f + (attackAnimationTime - 0.3f) * 8.0f;
        float slashAlpha = 1.0f - i * 0.2f - (attackAnimationTime - 0.3f) * 3.0f;

        if (slashAlpha > 0.0f && slashAlpha <= 1.0f) {
            Color swordEnergy(1.0f, 0.9f, 1.0f, slashAlpha);

            // Sword slash trail - positioned at sword location
            glPushMatrix();
            glTranslatef(0.8f, 0.8f, 0.0f); // Right hand position
            glRotatef(rightArmAngle, 1, 0, 0);
            glRotatef(-bodySway, 0, 0, 1);
            glTranslatef(0, -1.0f, 0);
            glRotatef(rightHandAngleX + swordAngleX, 1, 0, 0);
            glRotatef(rightHandAngleY + swordAngleY, 0, 1, 0);
            glRotatef(swordAngleZ, 0, 0, 1);
            glTranslatef(0, -0.8f, 0); // Move to sword blade
            glScalef(0.3f, slashLength, 0.3f);
            glColor4f(swordEnergy.r, swordEnergy.g, swordEnergy.b, swordEnergy.a);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}
void Ghost::updateShieldAnimation(float deltaTime) {
    if (isShieldActive) {
        glowIntensity = 2.0f + sin(animationTime * 8.0f) * 0.5f;
        ghostOpacity = 0.9f;
        addShieldParticles();
    }

    // Return to idle after shield animation
    if (stateTime > 1.0f && !isShieldActive) {
        currentState = IDLE;
        stateTime = 0.0f;
    }
}

void Ghost::updateHurtAnimation(float deltaTime) {
    // Flash red when hurt
    isFlashing = true;
    flashTime = 0.2f;

    // Return to idle quickly
    if (stateTime > 0.5f) {
        currentState = IDLE;
        stateTime = 0.0f;
    }
}

void Ghost::updateInvisibilityAnimation(float deltaTime) {
    // Fade in and out during invisibility
    float fadePattern = sin(animationTime * 8.0f);
    ghostOpacity = 0.0f + 0.05f * fadePattern;
    glowIntensity = 0.3f;

    // Still float while invisible
    floatOffset = sin(animationTime * 2.0f) * 0.3f;
}

void Ghost::render() {
    glPushMatrix();
    // Apply floating offset
    glTranslatef(x, y + floatOffset, z);
    // Apply rotation to face the target
    glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
    // Apply flash effect
    if (isFlashing) {
        float flashIntensity = sin(flashTime * 50.0f);
        glColor4f(1.0f, 0.2f, 0.2f, ghostOpacity * (0.5f + 0.5f * flashIntensity));
    }
    // Enable blending for ghost transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Draw ghost components
    drawGhostBody();
    drawGhostArms();
    drawGhostHands();
    if (hasSword) {
        drawGhostSword(); // Draw sword if equipped
    }
    drawGhostEyes();
    drawGhostTail();
    // Draw shield if active
    if (isShieldActive) {
        drawShield();
    }
    // Draw attack effect
    if (currentState == ATTACKING && attackAnimationTime > 0.3f && attackAnimationTime < 0.6f) {
        if (usingSword) {
            drawSwordAttackEffect();  // Sword attack effect
        } else {
            drawShadowSwipeEffect();  // Regular attack effect
        }
    }
    glDisable(GL_BLEND);
    glPopMatrix();
    // Draw particles (world space)
    drawParticles();
    // Draw health bar above ghost
    glPushMatrix();
    glTranslatef(x, y + 3.0f, z);
    drawHealthBar();
    glPopMatrix();
}
void Ghost::setSword(bool equipped) {
    hasSword = equipped;
    if (equipped) {
        attackDamage = 15.0f; // Sword does more damage
        attackRange = 3.5f;   // Longer reach with sword
    } else {
        attackDamage = 10.0f; // Default damage
        attackRange = 3.0f;   // Default range
    }
}
void Ghost::drawGhostBody() {
    // Set ghost material - SOLID for minecraft style
    Color ghostColor(0.2f, 0.2f, 0.2f, ghostOpacity); // Dark gray/black like minecraft
    setMaterial(Color(0.1f, 0.1f, 0.1f, ghostOpacity),
                ghostColor,
                Color(0.3f, 0.3f, 0.3f, ghostOpacity),
                10.0f);

    // Main body (minecraft-style rectangular body)
    glPushMatrix();
    glScalef(1.0f, 1.5f, 0.8f); // Rectangular body proportions
    glColor4f(ghostColor.r, ghostColor.g, ghostColor.b, ghostColor.a);
    glutSolidCube(1.6f); // Use cube instead of sphere
    glPopMatrix();

    // Head (cubic head like minecraft)
    glPushMatrix();
    glTranslatef(0.0f, 1.2f, 0.0f);
    glColor4f(ghostColor.r, ghostColor.g, ghostColor.b, ghostOpacity);
    glutSolidCube(1.2f); // Cubic head
    glPopMatrix();
}
void Ghost::drawGhostArms() {
    Color armColor(0.15f, 0.15f, 0.15f, ghostOpacity);
    setMaterial(Color(0.1f, 0.1f, 0.1f, ghostOpacity),
                armColor,
                Color(0.25f, 0.25f, 0.25f, ghostOpacity),
                8.0f);

    // Left arm
    glPushMatrix();
    glTranslatef(-0.8f, 0.8f, 0.0f);
    glRotatef(leftArmAngle, 1, 0, 0);
    glRotatef(bodySway, 0, 0, 1);
    glTranslatef(0, -0.5f, 0);
    glScalef(0.3f, 1.0f, 0.3f);
    glColor4f(armColor.r, armColor.g, armColor.b, armColor.a);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right arm
    glPushMatrix();
    glTranslatef(0.8f, 0.8f, 0.0f);
    glRotatef(rightArmAngle, 1, 0, 0);
    glRotatef(-bodySway, 0, 0, 1);
    glTranslatef(0, -0.5f, 0);
    glScalef(0.3f, 1.0f, 0.3f);
    glColor4f(armColor.r, armColor.g, armColor.b, armColor.a);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void Ghost::drawGhostHands() {
    Color handColor(0.25f, 0.25f, 0.25f, ghostOpacity);
    setMaterial(Color(0.15f, 0.15f, 0.15f, ghostOpacity),
                handColor,
                Color(0.4f, 0.4f, 0.4f, ghostOpacity),
                15.0f);

    // Left hand
    glPushMatrix();
    glTranslatef(-0.8f, 0.8f, 0.0f);
    glRotatef(leftArmAngle, 1, 0, 0);
    glRotatef(bodySway, 0, 0, 1);
    glTranslatef(0, -1.0f, 0);
    glRotatef(leftHandAngleX, 1, 0, 0);
    glRotatef(leftHandAngleY, 0, 1, 0);
    glRotatef(leftHandAngleZ, 0, 0, 1);
    glRotatef(leftHandAngle, 1, 0, 0);
    glScalef(0.35f, 0.35f, 0.5f);
    glColor4f(handColor.r, handColor.g, handColor.b, handColor.a);
    glutSolidCube(1.0f); // LEFT HAND CUBE
    glPopMatrix();

    // Right hand - FIX: Add the missing cube drawing
    glPushMatrix();
    glTranslatef(0.8f, 0.8f, 0.0f);
    glRotatef(rightArmAngle, 1, 0, 0);
    glRotatef(-bodySway, 0, 0, 1);
    glTranslatef(0, -1.0f, 0);
    glRotatef(rightHandAngleX, 1, 0, 0);
    glRotatef(rightHandAngleY, 0, 1, 0);
    glRotatef(rightHandAngleZ, 0, 0, 1);
    glRotatef(rightHandAngle, 1, 0, 0);
    glScalef(0.35f, 0.35f, 0.5f);
    glColor4f(handColor.r, handColor.g, handColor.b, handColor.a);
    glutSolidCube(1.0f); // RIGHT HAND CUBE - THIS WAS MISSING!
    glPopMatrix();
}
void Ghost::drawGhostEyes() {
    // Glowing RED eyes like minecraft ghost
    Color eyeColor(1.0f, 0.2f, 0.2f, 1.0f); // Bright red
    setMaterial(eyeColor, eyeColor, Color(1.0f, 0.5f, 0.5f), 50.0f);

    // Left eye (cubic style)
    glPushMatrix();
    glTranslatef(-0.25f, 1.35f, 0.55f); // Position on face
    glColor4f(eyeColor.r, eyeColor.g, eyeColor.b, eyeColor.a);
    glutSolidCube(0.15f); // Small cubic eyes
    glPopMatrix();

    // Right eye (cubic style)
    glPushMatrix();
    glTranslatef(0.25f, 1.35f, 0.55f);
    glColor4f(eyeColor.r, eyeColor.g, eyeColor.b, eyeColor.a);
    glutSolidCube(0.15f); // Small cubic eyes
    glPopMatrix();

    // Eye glow effect during attack
    if (currentState == ATTACKING) {
        glDisable(GL_LIGHTING);
        Color glowColor(1.0f, 0.1f, 0.1f, 0.5f); // Bright red glow

        // Left eye glow
        glPushMatrix();
        glTranslatef(-0.25f, 1.35f, 0.55f);
        glColor4f(glowColor.r, glowColor.g, glowColor.b, glowColor.a);
        glutSolidCube(0.4f); // Larger glow cube
        glPopMatrix();

        // Right eye glow
        glPushMatrix();
        glTranslatef(0.25f, 1.35f, 0.55f);
        glColor4f(glowColor.r, glowColor.g, glowColor.b, glowColor.a);
        glutSolidCube(0.4f); // Larger glow cube
        glPopMatrix();

        glEnable(GL_LIGHTING);
    }
}
void Ghost::drawGhostSword() {
    if (!hasSword) return;

    Color swordColor(0.9f, 0.9f, 1.0f, ghostOpacity);
    Color glowColor(1.0f, 0.9f, 1.0f, ghostOpacity * 0.8f);

    // Sword attached to right hand
    glPushMatrix();
    glTranslatef(0.8f, 0.8f, 0.0f); // Right hand position
    glRotatef(rightArmAngle, 1, 0, 0);
    glRotatef(-bodySway, 0, 0, 1);
    glTranslatef(0, -1.0f, 0); // Move to hand position

    // Apply hand rotations
    glRotatef(rightHandAngleX, 1, 0, 0);
    glRotatef(rightHandAngleY, 0, 1, 0);
    glRotatef(rightHandAngleZ, 0, 0, 1);
    glRotatef(rightHandAngle, 1, 0, 0);

    // Apply sword-specific rotations
    glRotatef(swordAngleX, 1, 0, 0);
    glRotatef(swordAngleY, 0, 1, 0);
    glRotatef(swordAngleZ, 0, 0, 1);

    // Sword glow effect during attack
    if (currentState == ATTACKING) {
        setMaterial(glowColor, glowColor, Color(1.0f, 1.0f, 1.0f), 100.0f);
        glColor4f(glowColor.r, glowColor.g, glowColor.b, glowColor.a);
    } else {
        setMaterial(swordColor, swordColor, Color(1.0f, 1.0f, 1.0f), 50.0f);
        glColor4f(swordColor.r, swordColor.g, swordColor.b, swordColor.a);
    }

    // Draw sword blade
    glPushMatrix();
    glTranslatef(0, -0.8f, 0); // Extend from hand
    glScalef(0.1f, 1.8f, 0.1f); // Long thin blade
    glutSolidCube(1.0f);
    glPopMatrix();

    // Sword hilt
    Color hiltColor(0.3f, 0.3f, 0.3f, ghostOpacity);
    setMaterial(hiltColor, Color(0.5f, 0.5f, 0.5f, ghostOpacity), Color(0.7f, 0.7f, 0.7f, ghostOpacity), 30.0f);
    glColor4f(0.5f, 0.5f, 0.5f, ghostOpacity);

    glPushMatrix();
    glTranslatef(0, -0.2f, 0);
    glScalef(0.15f, 0.3f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Crossguard
    glPushMatrix();
    glTranslatef(0, -0.35f, 0);
    glScalef(0.3f, 0.05f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}
void Ghost::drawGhostTail() {
    // Minecraft-style cubic tail segments
    Color tailColor(0.15f, 0.15f, 0.15f, ghostOpacity * 0.9f); // Dark gray
    setMaterial(Color(0.1f, 0.1f, 0.1f, ghostOpacity * 0.9f),
                tailColor,
                Color(0.2f, 0.2f, 0.2f, ghostOpacity * 0.9f),
                5.0f);

    // Draw cubic tail segments (like minecraft ghost)
    for (int i = 0; i < 4; i++) {
        float segmentY = -0.3f - i * 0.4f;
        float waveOffset = sin(animationTime * 3.0f + i * 0.5f) * 0.15f; // Subtle sway
        float segmentSize = 0.8f - i * 0.1f; // Gradually smaller

        glPushMatrix();
        glTranslatef(waveOffset, segmentY, 0.0f);
        glColor4f(tailColor.r, tailColor.g, tailColor.b, tailColor.a * (1.0f - i * 0.1f));
        glutSolidCube(segmentSize); // Cubic segments instead of spheres
        glPopMatrix();
    }
}

void Ghost::drawShield() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Shield sphere with pulsing effect
    float shieldPulse = 1.0f + 0.3f * sin(animationTime * 8.0f);
    Color shieldColor(0.4f, 0.8f, 1.0f, 0.3f);

    glPushMatrix();
    glScalef(shieldPulse, shieldPulse, shieldPulse);
    glColor4f(shieldColor.r, shieldColor.g, shieldColor.b, shieldColor.a);
    glutWireSphere(2.0f, 12, 8);
    glPopMatrix();

    // Inner glow
    glPushMatrix();
    glScalef(shieldPulse * 0.8f, shieldPulse * 0.8f, shieldPulse * 0.8f);
    glColor4f(shieldColor.r, shieldColor.g, shieldColor.b, shieldColor.a * 0.5f);
    glutSolidSphere(2.0f, 8, 6);
    glPopMatrix();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}

void Ghost::drawShadowSwipeEffect() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Dark energy waves
    for (int i = 0; i < 3; i++) {
        float waveRadius = 1.0f + i * 0.5f + (attackAnimationTime - 0.5f) * 10.0f;
        float waveAlpha = 0.6f - i * 0.2f - (attackAnimationTime - 0.5f) * 3.0f;

        if (waveAlpha > 0.0f) {
            Color darkEnergy(0.3f, 0.1f, 0.5f, waveAlpha);

            glPushMatrix();
            glRotatef(i * 45.0f, 0, 1, 0);
            glScalef(waveRadius, 0.2f, waveRadius);
            glColor4f(darkEnergy.r, darkEnergy.g, darkEnergy.b, darkEnergy.a);
            glutSolidTorus(0.1f, 1.0f, 6, 12);
            glPopMatrix();
        }
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}

void Ghost::drawHealthBar() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    float barWidth = 2.0f;
    float barHeight = 0.2f;
    float healthPercent = health / maxHealth;

    // Background
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-barWidth/2, 0, 0);
    glVertex3f(barWidth/2, 0, 0);
    glVertex3f(barWidth/2, barHeight, 0);
    glVertex3f(-barWidth/2, barHeight, 0);
    glEnd();

    // Health bar
    if (healthPercent > 0.6f) {
        glColor3f(0.2f, 0.8f, 0.2f); // Green
    } else if (healthPercent > 0.3f) {
        glColor3f(0.8f, 0.8f, 0.2f); // Yellow
    } else {
        glColor3f(0.8f, 0.2f, 0.2f); // Red
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

void Ghost::updateParticles(float deltaTime) {
    for (std::vector<GhostParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
        GhostParticle& p = *it;
        if (p.life > 0.0f) {
            p.x += p.vx * deltaTime;
            p.y += p.vy * deltaTime;
            p.z += p.vz * deltaTime;
            p.life -= deltaTime;
            p.opacity = p.life / p.maxLife;
        }
    }
}

void Ghost::addGhostTrailParticles() {
    for (int i = 0; i < 3; i++) {
        for (std::vector<GhostParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            GhostParticle& p = *it;
            if (p.life <= 0.0f) {
                p.x = x + ((rand() % 100) - 50) * 0.02f;
                p.y = y + floatOffset + ((rand() % 100) - 50) * 0.01f;
                p.z = z + ((rand() % 100) - 50) * 0.02f;
                p.vx = ((rand() % 100) - 50) * 0.005f;
                p.vy = ((rand() % 50) + 25) * 0.01f;
                p.vz = ((rand() % 100) - 50) * 0.005f;
                p.life = 1.0f + (rand() % 100) * 0.01f;
                p.maxLife = p.life;
                p.size = 0.05f + (rand() % 20) * 0.005f;
                p.opacity = 0.6f;
                break;
            }
        }
    }
}

void Ghost::addShieldParticles() {
    for (int i = 0; i < 2; i++) {
        for (std::vector<GhostParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            GhostParticle& p = *it;
            if (p.life <= 0.0f) {
                float angle = (rand() % 360) * 3.14159f / 180.0f;
                float radius = 2.0f + (rand() % 50) * 0.02f;

                p.x = x + cos(angle) * radius;
                p.y = y + floatOffset + ((rand() % 100) - 50) * 0.02f;
                p.z = z + sin(angle) * radius;
                p.vx = cos(angle) * 0.5f;
                p.vy = ((rand() % 50)) * 0.01f;
                p.vz = sin(angle) * 0.5f;
                p.life = 0.5f;
                p.maxLife = p.life;
                p.size = 0.1f;
                p.opacity = 0.8f;
                break;
            }
        }
    }
}

void Ghost::addAttackParticles() {
    for (int i = 0; i < 5; i++) {
        for (std::vector<GhostParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            GhostParticle& p = *it;
            if (p.life <= 0.0f) {
                float angle = (rand() % 360) * 3.14159f / 180.0f;
                float radius = (rand() % 100) * 0.05f;

                p.x = x + cos(angle) * radius;
                p.y = y + floatOffset;
                p.z = z + sin(angle) * radius;
                p.vx = cos(angle) * 5.0f;
                p.vy = ((rand() % 100) - 50) * 0.02f;
                p.vz = sin(angle) * 5.0f;
                p.life = 0.3f;
                p.maxLife = p.life;
                p.size = 0.15f;
                p.opacity = 1.0f;
                break;
            }
        }
    }
}

void Ghost::addDashParticles() {
    for (int i = 0; i < 4; i++) {
        for (std::vector<GhostParticle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            GhostParticle& p = *it;
            if (p.life <= 0.0f) {
                p.x = x + ((rand() % 100) - 50) * 0.03f;
                p.y = y + floatOffset + ((rand() % 100) - 50) * 0.02f;
                p.z = z + ((rand() % 100) - 50) * 0.03f;
                p.vx = ((rand() % 200) - 100) * 0.01f;
                p.vy = ((rand() % 100)) * 0.005f;
                p.vz = ((rand() % 200) - 100) * 0.01f;
                p.life = 0.8f;
                p.maxLife = p.life;
                p.size = 0.1f;
                p.opacity = 0.9f;
                break;
            }
        }
    }
}

void Ghost::drawParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (std::vector<GhostParticle>::const_iterator it = particles.begin(); it != particles.end(); ++it) {
    const GhostParticle& p = *it;
    if (p.life > 0.0f) {
        // Different colors based on particle type
        if (p.type == 4) {
            // Sword dash particles - bright silver/white with blue tint
            glColor4f(0.9f, 0.9f, 1.0f, p.opacity);
        } else if (p.type == 3) {
            // Sword attack particles - bright white/silver
            glColor4f(1.0f, 0.95f, 1.0f, p.opacity);
        } else if (isShieldActive && p.maxLife < 0.6f) {
            glColor4f(0.4f, 0.8f, 1.0f, p.opacity * 0.8f); // Shield particles
        } else if (currentState == ATTACKING && p.maxLife < 0.4f) {
            glColor4f(0.5f, 0.2f, 0.8f, p.opacity); // Attack particles
        } else {
            glColor4f(0.8f, 0.8f, 1.0f, p.opacity * 0.6f); // Trail particles
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

void Ghost::setMaterial(const Color& ambient, const Color& diffuse, const Color& specular, float shininess) {
    GLfloat mat_ambient[] = {ambient.r, ambient.g, ambient.b, ambient.a};
    GLfloat mat_diffuse[] = {diffuse.r, diffuse.g, diffuse.b, diffuse.a};
    GLfloat mat_specular[] = {specular.r, specular.g, specular.b, specular.a};

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void Ghost::takeDamage(float damage) {
    if (isShieldActive) {

        return;
    }

    health -= damage;
    if (health < 0.0f) health = 0.0f;

    currentState = HURT;
    stateTime = 0.0f;


}

void Ghost::activateShield() {
    if (shieldCooldown <= 0.0f && !isShieldActive) {
        isShieldActive = true;
        shieldDuration = 3.0f; // Shield lasts 3 seconds
        currentState = SHIELDING;
        stateTime = 0.0f;


    }
}

bool Ghost::canDash() {
    return dashCooldown <= 0.0f && !isDashing && currentState != ATTACKING;
}

void Ghost::dash(float newTargetX, float newTargetZ) {
    if (canDash()) {
        float currentDistance = sqrt((newTargetX - x) * (newTargetX - x) + (newTargetZ - z) * (newTargetZ - z));
        if (currentDistance < 1.0f) {
            return;
        }

        // Calculate direction to target
        float dirX = newTargetX - x;
        float dirZ = newTargetZ - z;
        float distance = sqrt(dirX * dirX + dirZ * dirZ);

        // Set target to stop before collision (1.5f units away from slime)
        if (distance > 1.5f) {
            float normalizedDirX = dirX / distance;
            float normalizedDirZ = dirZ / distance;
            targetX = newTargetX - (normalizedDirX * 1.5f);
            targetZ = newTargetZ - (normalizedDirZ * 1.5f);
        } else {
            targetX = newTargetX;
            targetZ = newTargetZ;
        }

        isDashing = true;
        dashDuration = 0.8f;
        currentState = DASHING;
        stateTime = 0.0f;
    }
}

bool Ghost::canAttack() {
    return attackCooldown <= 0.0f && !isAttacking && !isDashing;
}

void Ghost::shadowSwipe(float targetX, float targetZ) {
    if (canAttack()) {
        isAttacking = true;
        currentState = ATTACKING;
        stateTime = 0.0f;
        attackAnimationTime = 0.0f;
        swordSwingProgress = 0.0f;

        // Face the target
        this->targetX = targetX;
        this->targetZ = targetZ;
    }
}

bool Ghost::canGoInvisible() {
    return invisibilityCooldown <= 0.0f && !isInvisible && currentState != ATTACKING && !isDashing;
}

void Ghost::goInvisible() {
    if (canGoInvisible()) {
        isInvisible = true;
        invisibilityDuration = 4.0f; // Invisible for 4 seconds
        currentState = INVISIBLE;
        stateTime = 0.0f;


    }
}

float Ghost::distanceTo(float tx, float tz) {
    float dx = tx - x;
    float dz = tz - z;
    return sqrt(dx * dx + dz * dz);
}

void Ghost::setPosition(float newX, float newZ) {
    x = newX;
    z = newZ;
}

void Ghost::setTarget(float newTargetX, float newTargetZ) {
    targetX = newTargetX;
    targetZ = newTargetZ;
}

void Ghost::moveTowards(float targetX, float targetZ, float speed) {
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

bool Ghost::isInRange(float targetX, float targetZ, float range) {
    return distanceTo(targetX, targetZ) <= range;
}

void Ghost::reset() {
    health = maxHealth;
    isShieldActive = false;
    shieldDuration = 0.0f;
    shieldCooldown = 0.0f;
    isDashing = false;
    dashCooldown = 0.0f;
    isAttacking = false;
    attackCooldown = 0.0f;
    isInvisible = false;
    invisibilityDuration = 0.0f;
    invisibilityCooldown = 0.0f;
    currentState = IDLE;
    animationTime = 0.0f;
    stateTime = 0.0f;
    isFlashing = false;
    flashTime = 0.0f;
    ghostOpacity = 0.95f; // Reset to solid


}
