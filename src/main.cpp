#include <GL/glut.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include "Environment.hpp"
#include "Ghost.hpp"
#include "Slime.hpp"

// Global variables
Environment* environment = NULL;
Ghost* ghost = NULL;
Slime* slime = NULL;
int windowWidth = 1024;
int windowHeight = 768;
bool keys[256] = {false};

// Camera controls - Direct position based
struct CameraSettings {
    float posInc = 1.0f;  // Position increment for movement
} setting;

struct Camera {
    float x = 0.0f;
    float y = 15.0f;      // Changed from 10.0f - higher up
    float z = 13.0f;      // Changed from 20.0f - closer to center
    float lookAtX = 0.0f;
    float lookAtY = 0.0f; // Changed from 2.0f - looking down at ground level
    float lookAtZ = 0.0f;

    void move(float xinc, float yinc, float zinc) {
        x += xinc;
        y += yinc;
        z += zinc;
    }

    void reset() {
    x = 0.0f;
    y = 15.0f;    // Changed from 10.0f
    z = 12.0f;    // Changed from 20.0f
    }
} camera;


// Battle system
enum GameState {
    BATTLE_ACTIVE,
    GHOST_WINS,
    SLIME_WINS,
    GAME_PAUSED
};
GameState currentGameState = BATTLE_ACTIVE;

// Arena boundaries
const float ARENA_SIZE = 5.0f;
float battleTimer = 0.0f;

// Damage tracking for hit prevention
bool ghostHasHitSlime = false;
float ghostHitCooldown = 0.0f;

// Slime movement control
const float SLIME_MOVE_SPEED = 0.5f;
// Slime knockback system
struct SlimeKnockback {
    bool isKnockedBack = false;
    float knockbackVelX = 0.0f;
    float knockbackVelZ = 0.0f;
    float knockbackDuration = 0.0f;
    const float KNOCKBACK_FORCE = 3.0f;
    const float KNOCKBACK_TIME = 0.3f;
    const float FRICTION = 0.9f;
} slimeKnockback;
// Helper function to calculate rotation angle to face target
float calculateRotationToFace(float fromX, float fromZ, float toX, float toZ) {
    float dx = toX - fromX;
    float dz = toZ - fromZ;
    return atan2(dx, dz) * 180.0f / M_PI; // Convert to degrees
}
const float COLLISION_DISTANCE = 1.5f; // Distance to stop before slime
// Combat system
void checkCollisions();
void updateBattleSystem(float deltaTime);
void restrictToBounds(float& x, float& z);
void drawBattleUI();
void resetBattle();
void updateSlimeKnockback(float deltaTime);
void applyKnockbackToSlime(float ghostX, float ghostZ, float slimeX, float slimeZ);
void initOpenGL() {
    // Set clear color to sky blue
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable smooth shading
    glShadeModel(GL_SMOOTH);

    // Enable back-face culling for better performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Set up perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)windowWidth / (double)windowHeight, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);


    // Initialize environment and characters
    environment = new Environment();
    environment->initialize();

    // Position characters on opposite sides of the arena
    ghost = new Ghost(-3.0f, -3.0f);
    slime = new Slime(3.0f, 3.0f);

    std::cout << "=== BLIND BOX BATTLE ARENA ===" << std::endl;
    std::cout << "Ghost vs Slime - BALANCED Battle!" << std::endl;
    std::cout << "========== CONTROLS ==========" << std::endl;
    std::cout << "Camera:" << std::endl;
    std::cout << "  A/D      - Move camera left/right" << std::endl;
    std::cout << "  Q/E      - Move camera down/up" << std::endl;
    std::cout << "  W/S      - Move camera forward/back" << std::endl;
    std::cout << "  R        - Reset camera position" << std::endl;
    std::cout << "  L        - Cycle lighting" << std::endl;
    std::cout << "======= GHOST CONTROLS =======" << std::endl;
    std::cout << "  1        - Ghost dash to slime (8 dmg)" << std::endl;
    std::cout << "  2        - Ghost shadow swipe (10 dmg)" << std::endl;
    std::cout << "  3        - Ghost shield" << std::endl;
    std::cout << "  4        - Ghost invisibility" << std::endl;
    std::cout << "======= SLIME CONTROLS =======" << std::endl;
    std::cout << "  6        - Slime bomb at ghost (12 dmg)" << std::endl;
    std::cout << "  7        - Slime shield" << std::endl;
    std::cout << "  8        - Slime bounce dodge" << std::endl;
    std::cout << "  Arrow Keys - Move slime" << std::endl;
    std::cout << "======= BATTLE SYSTEM =======" << std::endl;
    std::cout << "  5        - Reset battle" << std::endl;
    std::cout << "  0        - Pause/Resume" << std::endl;
    std::cout << "  ESC      - Exit" << std::endl;
    std::cout << "=============================" << std::endl;
    std::cout << "BALANCE: Ghost=100hp, Slime=120hp" << std::endl;
}

void display() {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set camera using direct position
    gluLookAt(camera.x, camera.y, camera.z,
              camera.lookAtX, camera.lookAtY, camera.lookAtZ,
              0.0, 1.0, 0.0);

    // Render environment
    if (environment) {
        environment->render();
    }

    // Render characters
    if (ghost) {
        ghost->render();
    }

    if (slime) {
        slime->render();
    }

    // Draw battle UI
    drawBattleUI();

    glutSwapBuffers();
}

void reshape(int width, int height) {
    windowWidth = width;
    windowHeight = height;

    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / (double)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void restrictToBounds(float& x, float& z) {
    if (x > ARENA_SIZE) x = ARENA_SIZE;
    if (x < -ARENA_SIZE) x = -ARENA_SIZE;
    if (z > ARENA_SIZE) z = ARENA_SIZE;
    if (z < -ARENA_SIZE) z = -ARENA_SIZE;
}

void checkCollisions() {
    if (!ghost || !slime) return;

    float ghostX = ghost->getX();
    float ghostZ = ghost->getZ();
    float slimeX = slime->getX();
    float slimeZ = slime->getZ();
    float distance = sqrt((ghostX - slimeX) * (ghostX - slimeX) + (ghostZ - slimeZ) * (ghostZ - slimeZ));

    // Update ghost hit cooldown
    if (ghostHitCooldown > 0.0f) {
        ghostHitCooldown -= 0.016f; // Assuming 60 FPS
    }

    // Reset hit flag when ghost stops attacking/dashing
    if (!ghost->getIsDashing() && !ghost->getIsAttacking()) {
        ghostHasHitSlime = false;
    }

    // Check if ghost dash hits slime
    if (ghost->getIsDashing() && distance <= 2.0f && !ghostHasHitSlime && ghostHitCooldown <= 0.0f) {
    slime->takeDamage(8.0f);
    ghostHasHitSlime = true;
    ghostHitCooldown = 1.0f;

    // Apply knockback to slime
    applyKnockbackToSlime(ghostX, ghostZ, slimeX, slimeZ);

    // Add impact effect
    if (environment) {
        environment->addMagicEffect(slimeX, slimeZ);
    }
}

    // Check if ghost shadow swipe hits slime
    if (ghost->getIsAttacking() && distance <= 4.0f && !ghostHasHitSlime && ghostHitCooldown <= 0.0f) {
        slime->takeDamage(10.0f);
        ghostHasHitSlime = true;
        ghostHitCooldown = 1.0f;

        // Add impact effect
        if (environment) {
            environment->addMagicEffect(slimeX, slimeZ);
        }
    }

    // Check if slime bombs hit ghost
    if (slime->checkBombCollisions(ghostX, ghostZ, 2.5f)) {
        ghost->takeDamage(12.0f);

        // Add explosion effect
        if (environment) {
            environment->addMagicEffect(ghostX, ghostZ);
        }
    }

    // Check win conditions
    if (ghost->getHealth() <= 0 && currentGameState == BATTLE_ACTIVE) {
        currentGameState = SLIME_WINS;
        std::cout << "\n*** SLIME WINS THE BATTLE! ***" << std::endl;
        std::cout << "The green warrior triumphs!" << std::endl;
    } else if (slime->getHealth() <= 0 && currentGameState == BATTLE_ACTIVE) {
        currentGameState = GHOST_WINS;
        std::cout << "\n*** GHOST WINS THE BATTLE! ***" << std::endl;
        std::cout << "The shadow phantom prevails!" << std::endl;
    }
}

void updateBattleSystem(float deltaTime) {
    battleTimer += deltaTime;
    updateSlimeKnockback(deltaTime);
    if (currentGameState != BATTLE_ACTIVE) return;

    // Restrict characters to arena bounds
    if (ghost) {
        float ghostX = ghost->getX();
        float ghostZ = ghost->getZ();
        restrictToBounds(ghostX, ghostZ);
        ghost->setPosition(ghostX, ghostZ);
    }

    if (slime) {
        float slimeX = slime->getX();
        float slimeZ = slime->getZ();
        restrictToBounds(slimeX, slimeZ);
        slime->setPosition(slimeX, slimeZ);
    }
}

void drawBattleUI() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Battle status text (simple colored rectangles for health)
    // Ghost health bar (top left)
    float ghostHealthPercent = ghost ? (ghost->getHealth() / ghost->getMaxHealth()) : 0.0f;
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(20, windowHeight - 40);
    glVertex2f(220, windowHeight - 40);
    glVertex2f(220, windowHeight - 20);
    glVertex2f(20, windowHeight - 20);
    glEnd();

    glColor3f(0.8f, 0.2f, 0.8f); // Purple for ghost
    glBegin(GL_QUADS);
    glVertex2f(20, windowHeight - 40);
    glVertex2f(20 + 200 * ghostHealthPercent, windowHeight - 40);
    glVertex2f(20 + 200 * ghostHealthPercent, windowHeight - 20);
    glVertex2f(20, windowHeight - 20);
    glEnd();

    // Slime health bar (top right)
    float slimeHealthPercent = slime ? (slime->getHealth() / slime->getMaxHealth()) : 0.0f;
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(windowWidth - 220, windowHeight - 40);
    glVertex2f(windowWidth - 20, windowHeight - 40);
    glVertex2f(windowWidth - 20, windowHeight - 20);
    glVertex2f(windowWidth - 220, windowHeight - 20);
    glEnd();

    glColor3f(0.2f, 0.8f, 0.2f); // Green for slime
    glBegin(GL_QUADS);
    glVertex2f(windowWidth - 220, windowHeight - 40);
    glVertex2f(windowWidth - 220 + 200 * slimeHealthPercent, windowHeight - 40);
    glVertex2f(windowWidth - 220 + 200 * slimeHealthPercent, windowHeight - 20);
    glVertex2f(windowWidth - 220, windowHeight - 20);
    glEnd();

    // Character name labels under health bars
    // Ghost name under ghost health bar
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(25, windowHeight - 55);
    const char* ghostText = "GHOST";
    for (const char* c = ghostText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Slime name under slime health bar
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(windowWidth - 215, windowHeight - 55);
    const char* slimeText = "SLIME";
    for (const char* c = slimeText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Game Over text display
    if (currentGameState == GHOST_WINS || currentGameState == SLIME_WINS) {
        // Background for "GAME OVER" text - expanded upward
        glColor4f(0.0f, 0.0f, 0.0f, 0.8f); // Semi-transparent black
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glVertex2f(windowWidth/2 - 120, windowHeight/2 + 30);
        glVertex2f(windowWidth/2 + 120, windowHeight/2 + 30);
        glVertex2f(windowWidth/2 + 120, windowHeight/2 + 90);
        glVertex2f(windowWidth/2 - 120, windowHeight/2 + 90);
        glEnd();

        // Big "GAME OVER" text
        glColor3f(1.0f, 1.0f, 1.0f); // White color
        glRasterPos2f(windowWidth/2 - 80, windowHeight/2 + 60);
        const char* gameOverText = "GAME OVER";
        for (const char* c = gameOverText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }

        // Winner announcement below GAME OVER
        const char* winnerText;

        if (currentGameState == GHOST_WINS) {
            winnerText = "GHOST WINS!";
        } else {
            winnerText = "SLIME WINS!";
        }

        // Background for winner text - expanded upward
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(windowWidth/2 - 100, windowHeight/2 - 5);
        glVertex2f(windowWidth/2 + 100, windowHeight/2 - 5);
        glVertex2f(windowWidth/2 + 100, windowHeight/2 + 45);
        glVertex2f(windowWidth/2 - 100, windowHeight/2 + 45);
        glEnd();

        // Winner text with appropriate color
        if (currentGameState == GHOST_WINS) {
            glColor3f(0.8f, 0.2f, 0.8f); // Purple for ghost
        } else {
            glColor3f(0.2f, 0.8f, 0.2f); // Green for slime
        }

        glRasterPos2f(windowWidth/2 - 70, windowHeight/2 + 20);
        for (const char* c = winnerText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        }

        // Background for instruction texts - expanded upward
        glColor4f(0.0f, 0.0f, 0.0f, 0.6f); // Semi-transparent black
        glBegin(GL_QUADS);
        glVertex2f(windowWidth/2 - 130, windowHeight/2 - 50);
        glVertex2f(windowWidth/2 + 130, windowHeight/2 - 50);
        glVertex2f(windowWidth/2 + 130, windowHeight/2 + 10);
        glVertex2f(windowWidth/2 - 130, windowHeight/2 + 10);
        glEnd();
        glDisable(GL_BLEND);

        // Small instruction texts
        glColor3f(0.9f, 0.9f, 0.9f); // Light gray color
        glRasterPos2f(windowWidth/2 - 100, windowHeight/2 - 10);
        const char* resetText = "Click 5 to RESET BATTLE";
        for (const char* c = resetText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }

        glRasterPos2f(windowWidth/2 - 70, windowHeight/2 - 25);
        const char* exitText = "Click ESC TO Exit";
        for (const char* c = exitText; *c != '\0'; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        }
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}
void resetBattle() {
    if (ghost) {
        ghost->reset();
        ghost->setPosition(-3.0f, -3.0f);
    }
    if (slime) {
        slime->reset();
        slime->setPosition(3.0f, 3.0f);
    }
    currentGameState = BATTLE_ACTIVE;
    battleTimer = 0.0f;
    ghostHasHitSlime = false;
    ghostHitCooldown = 0.0f;
}

void keyboard(unsigned char key, int x, int y) {
    keys[key] = true;

    GLfloat xinc, yinc, zinc;
    xinc = yinc = zinc = 0.0;

    switch (key) {
        case 27: // ESC key
            if (environment) delete environment;
            if (ghost) delete ghost;
            if (slime) delete slime;
            exit(0);
            break;

        // Camera controls using direct position movement
        case 'a': case 'A': xinc = -setting.posInc; break;
        case 'd': case 'D': xinc =  setting.posInc; break;
        case 'q': case 'Q': yinc = -setting.posInc; break;
        case 'e': case 'E': yinc =  setting.posInc; break;
        case 'w': case 'W': zinc = -setting.posInc; break;
        case 's': case 'S': zinc =  setting.posInc; break;

        case 'r': case 'R':
            camera.reset();
            break;
        case 'l': case 'L':
            if (environment) environment->cycleLighting();
            break;

        // Ghost controls (1-4)
        case '1':
            if (ghost && ghost->canDash() && currentGameState == BATTLE_ACTIVE) {
                ghost->dash(slime->getX(), slime->getZ());
            }
            break;
        case '2':

            if (ghost && ghost->canAttack() && currentGameState == BATTLE_ACTIVE) {
                ghost->shadowSwipe(slime->getX(), slime->getZ());
            }
            break;
        case '3':
            if (ghost && currentGameState == BATTLE_ACTIVE) {
                ghost->activateShield();
            }
            break;
        case '4':
            if (ghost && ghost->canGoInvisible() && currentGameState == BATTLE_ACTIVE) {
                ghost->goInvisible();
            }
            break;

        // Battle system controls
        case '5':
            resetBattle();
            break;

        // Slime controls (6-8)
        case '6':
            if (slime && slime->canThrow() && currentGameState == BATTLE_ACTIVE) {
                slime->throwSlimeBomb(ghost->getX(), ghost->getZ());
            }
            break;
        case '7':
            if (slime && currentGameState == BATTLE_ACTIVE) {
                slime->activateShield();
            }
            break;
        case '8':
            if (slime && currentGameState == BATTLE_ACTIVE) {
                slime->bounce();
            }
            break;

        case '0':
            if (currentGameState == BATTLE_ACTIVE) {
                currentGameState = GAME_PAUSED;
            } else if (currentGameState == GAME_PAUSED) {
                currentGameState = BATTLE_ACTIVE;
            }
            break;
    }

    // Apply camera movement
    camera.move(xinc, yinc, zinc);

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    // Slime movement with arrow keys - Updated for directional movement
    if (slime && currentGameState == BATTLE_ACTIVE) {
        float newX = slime->getX();
        float newZ = slime->getZ();

        switch (key) {
            case GLUT_KEY_UP:
                // Move forward in the direction slime is facing
                {
                    float slimeRotRad = slime->getRotation() * M_PI / 180.0f;
                    newX += sin(slimeRotRad) * SLIME_MOVE_SPEED;
                    newZ += cos(slimeRotRad) * SLIME_MOVE_SPEED;
                }
                break;
            case GLUT_KEY_DOWN:
                // Move backward (opposite to facing direction)
                {
                    float slimeRotRad = slime->getRotation() * M_PI / 180.0f;
                    newX -= sin(slimeRotRad) * SLIME_MOVE_SPEED;
                    newZ -= cos(slimeRotRad) * SLIME_MOVE_SPEED;
                }
                break;
            case GLUT_KEY_LEFT:
                // Strafe left
                {
                    float slimeRotRad = slime->getRotation() * M_PI / 180.0f;
                    newX += cos(slimeRotRad) * SLIME_MOVE_SPEED;
                    newZ -= sin(slimeRotRad) * SLIME_MOVE_SPEED;
                }
                break;
            case GLUT_KEY_RIGHT:
                // Strafe right
                {
                    float slimeRotRad = slime->getRotation() * M_PI / 180.0f;
                    newX -= cos(slimeRotRad) * SLIME_MOVE_SPEED;
                    newZ += sin(slimeRotRad) * SLIME_MOVE_SPEED;
                }
                break;
        }

        restrictToBounds(newX, newZ);
        slime->setPosition(newX, newZ);
    }

    glutPostRedisplay();
}

void timer(int value) {
    float deltaTime = 0.016f;

    // Update environment
    if (environment) {
        environment->update(deltaTime);
    }

    // Update characters
    if (currentGameState == BATTLE_ACTIVE || currentGameState == GAME_PAUSED) {
        if (ghost) ghost->update(deltaTime);
        if (slime) slime->update(deltaTime);

        if (currentGameState == BATTLE_ACTIVE) {
            updateBattleSystem(deltaTime);
            checkCollisions();
        }
    }

    if (ghost && slime) {
    // Make ghost face slime
    float ghostRotation = calculateRotationToFace(
        ghost->getX(), ghost->getZ(),
        slime->getX(), slime->getZ()
    );
    ghost->setRotation(ghostRotation);

    // Make slime face ghost
    float slimeRotation = calculateRotationToFace(
        slime->getX(), slime->getZ(),
        ghost->getX(), ghost->getZ()
    );
    slime->setRotation(slimeRotation);
}

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Click to target functionality can be added here if needed
    }
}
void updateSlimeKnockback(float deltaTime) {
    if (slimeKnockback.isKnockedBack) {
        // Update knockback duration
        slimeKnockback.knockbackDuration -= deltaTime;

        if (slimeKnockback.knockbackDuration <= 0.0f) {
            // End knockback
            slimeKnockback.isKnockedBack = false;
            slimeKnockback.knockbackVelX = 0.0f;
            slimeKnockback.knockbackVelZ = 0.0f;
        } else {
            // Apply knockback movement
            float newX = slime->getX() + slimeKnockback.knockbackVelX * deltaTime;
            float newZ = slime->getZ() + slimeKnockback.knockbackVelZ * deltaTime;

            // Keep within bounds
            restrictToBounds(newX, newZ);
            slime->setPosition(newX, newZ);

            // Apply friction
            slimeKnockback.knockbackVelX *= slimeKnockback.FRICTION;
            slimeKnockback.knockbackVelZ *= slimeKnockback.FRICTION;
        }
    }
}

void applyKnockbackToSlime(float ghostX, float ghostZ, float slimeX, float slimeZ) {
    // Calculate knockback direction (from ghost to slime)
    float dirX = slimeX - ghostX;
    float dirZ = slimeZ - ghostZ;

    // Normalize direction
    float length = sqrt(dirX * dirX + dirZ * dirZ);
    if (length > 0.0f) {
        dirX /= length;
        dirZ /= length;
    }

    // Apply knockback
    slimeKnockback.isKnockedBack = true;
    slimeKnockback.knockbackVelX = dirX * slimeKnockback.KNOCKBACK_FORCE;
    slimeKnockback.knockbackVelZ = dirZ * slimeKnockback.KNOCKBACK_FORCE;
    slimeKnockback.knockbackDuration = slimeKnockback.KNOCKBACK_TIME;
}
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("TCG6223 - Balanced Blind Box Battle Arena");

    initOpenGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();

    // Cleanup
    if (environment) delete environment;
    if (ghost) delete ghost;
    if (slime) delete slime;

    return 0;
}
