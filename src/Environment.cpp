#include "Environment.hpp"
#include <cstdlib>
#include <ctime>
#include <cmath>

Environment::Environment() : initialized(false), time(0.0f), lightIntensity(1.0f), currentLightMode(0) {
    srand(42); // Fixed seed for consistent generation
}

Environment::~Environment() {
    // Cleanup if needed
}

void Environment::initialize() {
    setupLighting();
    generateStaticContent();
    initialized = true;
}

void Environment::generateStaticContent() {
    // Generate trees once
    trees.clear();
    for (int i = 0; i < 15; i++) {
        float x = (rand() % (FLOOR_SIZE * 2)) - FLOOR_SIZE;
        float z = (rand() % (FLOOR_SIZE * 2)) - FLOOR_SIZE;

        // Avoid center area (battle arena) - use fabs for float comparison
        if (fabs(x) < 8 && fabs(z) < 8) continue;

        float height = 3.0f + (rand() % 4);
        int type = rand() % 2;
        trees.push_back(TreeData(x, z, height, type));
    }

    // Generate grass patches once
    grassPatches.clear();
    for (int i = 0; i < 80; i++) {
        float x = (rand() % (FLOOR_SIZE * 2)) - FLOOR_SIZE;
        float z = (rand() % (FLOOR_SIZE * 2)) - FLOOR_SIZE;

        // Avoid center battle area - use fabs for float comparison
        if (fabs(x) < 7 && fabs(z) < 7) continue;

        float height = 0.3f + (rand() % 3) * 0.2f;
        Color grassColor;

        int colorType = rand() % 4;
        switch(colorType) {
            case 0: grassColor = Color(0.2f, 0.7f, 0.1f); break; // Bright green
            case 1: grassColor = Color(0.15f, 0.5f, 0.08f); break; // Dark green
            case 2: grassColor = Color(0.25f, 0.8f, 0.15f); break; // Light green
            default: grassColor = Color(0.1f, 0.4f, 0.05f); break; // Forest green
        }

        grassPatches.push_back(GrassData(x, z, height, grassColor));
    }

    // Generate clouds once
    clouds.clear();
    for (int i = 0; i < 12; i++) {
        float x = (rand() % (SKYBOX_SIZE)) - SKYBOX_SIZE/2;
        float z = (rand() % (SKYBOX_SIZE)) - SKYBOX_SIZE/2;
        float y = 15.0f + (rand() % 10);
        float size = 1.5f + (rand() % 3) * 0.5f;

        clouds.push_back(CloudData(x, y, z, size));
    }

    // Generate fixed floor pattern
    floorColors.clear();
    floorColors.resize(FLOOR_SIZE * 2);
    for (int i = 0; i < FLOOR_SIZE * 2; i++) {
        floorColors[i].resize(FLOOR_SIZE * 2);
        for (int j = 0; j < FLOOR_SIZE * 2; j++) {
            // Create interesting floor pattern
            if ((i + j) % 3 == 0) {
                floorColors[i][j] = Color(0.25f, 0.6f, 0.15f); // Light green
            } else if ((i + j) % 5 == 0) {
                floorColors[i][j] = Color(0.4f, 0.3f, 0.2f); // Dirt
            } else if ((i * j) % 7 == 0) {
                floorColors[i][j] = Color(0.3f, 0.7f, 0.2f); // Medium green
            } else {
                floorColors[i][j] = Color(0.2f, 0.5f, 0.12f); // Dark green
            }
        }
    }

    // Initialize particle system
    particles.resize(200);
}

void Environment::setupLighting() {
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE); // Important for proper lighting

    // Stable ambient light
    GLfloat ambient[] = {0.4f, 0.4f, 0.5f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    // Main sun light (directional) - FIXED position
    GLfloat light0_pos[] = {1.0f, 1.0f, 0.5f, 0.0f}; // Directional light
    GLfloat light0_diffuse[] = {0.9f, 0.9f, 0.8f, 1.0f};
    GLfloat light0_specular[] = {1.0f, 1.0f, 0.9f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    // Secondary fill light - FIXED position
    GLfloat light1_pos[] = {-0.5f, 0.8f, -0.3f, 0.0f};
    GLfloat light1_diffuse[] = {0.5f, 0.6f, 0.8f, 1.0f};
    GLfloat light1_specular[] = {0.3f, 0.4f, 0.6f, 1.0f};

    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

    // Enable color material for easy color changes
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void Environment::setMaterial(const Color& ambient, const Color& diffuse, const Color& specular, float shininess) {
    GLfloat mat_ambient[] = {ambient.r, ambient.g, ambient.b, 1.0f};
    GLfloat mat_diffuse[] = {diffuse.r, diffuse.g, diffuse.b, 1.0f};
    GLfloat mat_specular[] = {specular.r, specular.g, specular.b, 1.0f};

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void Environment::drawVoxelCube(float x, float y, float z, float size, const Color& color) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(size, size, size);

    glColor3f(color.r, color.g, color.b);

    // Draw cube faces with proper normals for lighting
    glBegin(GL_QUADS);

    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);

    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);

    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);

    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);

    // Top face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);

    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);

    glEnd();
    glPopMatrix();
}

void Environment::drawFloor() {
    // Draw stable floor using pre-generated pattern
    setMaterial(Color(0.2f, 0.4f, 0.1f), Color(0.3f, 0.6f, 0.2f), Color(0.1f, 0.1f, 0.1f), 10.0f);

    for (int x = -FLOOR_SIZE; x < FLOOR_SIZE; x++) {
        for (int z = -FLOOR_SIZE; z < FLOOR_SIZE; z++) {
            int arrayX = x + FLOOR_SIZE;
            int arrayZ = z + FLOOR_SIZE;

            // Use pre-generated colors
            Color grassColor = floorColors[arrayX][arrayZ];

            // Add slight height variation but keep it stable
            float height = 0.1f + ((arrayX + arrayZ) % 3) * 0.03f;

            drawVoxelCube(x, -height/2, z, 1.0f, grassColor);
        }
    }
}

void Environment::drawBattleArena() {
    // Draw battle arena (clearer center area)
    setMaterial(Color(0.3f, 0.3f, 0.3f), Color(0.6f, 0.6f, 0.7f), Color(0.4f, 0.4f, 0.5f), 30.0f);

    // Arena border with glowing effect
    for (int x = -6; x <= 6; x++) {
        for (int z = -6; z <= 6; z++) {
            if (x == -6 || x == 6 || z == -6 || z == 6) {
                // Glowing border stones
                Color stoneColor(0.7f + 0.2f * sin(time * 2.0f),
                               0.7f + 0.1f * sin(time * 3.0f),
                               0.8f + 0.15f * sin(time * 1.5f));
                drawVoxelCube(x, 0.3f, z, 1.0f, stoneColor);

                // Add small light pillars at corners
                if ((x == -6 || x == 6) && (z == -6 || z == 6)) {
                    Color lightColor(1.0f, 0.9f, 0.7f);
                    drawVoxelCube(x, 1.0f, z, 0.3f, lightColor);
                    drawVoxelCube(x, 1.7f, z, 0.2f, lightColor);
                }
            }
        }
    }

    // Arena floor pattern
    for (int x = -5; x <= 5; x++) {
        for (int z = -5; z <= 5; z++) {
            Color floorColor;
            if ((x + z) % 2 == 0) {
                floorColor = Color(0.5f, 0.5f, 0.6f); // Light stone
            } else {
                floorColor = Color(0.4f, 0.4f, 0.5f); // Dark stone
            }
            drawVoxelCube(x, 0.05f, z, 1.0f, floorColor);
        }
    }
}

void Environment::drawSkybox() {
    glPushMatrix();
    glDisable(GL_LIGHTING); // Skybox shouldn't be affected by lighting

    float size = SKYBOX_SIZE;

    // Sky blue gradient effect using multiple quads
    for (int layer = 0; layer < 5; layer++) {
        float y = size * 0.3f + layer * 2.0f;
        float intensity = 1.0f - (layer * 0.15f);
        Color skyColor(0.4f * intensity, 0.7f * intensity, 1.0f * intensity);

        glColor3f(skyColor.r, skyColor.g, skyColor.b);
        glBegin(GL_QUADS);
        glVertex3f(-size, y, -size);
        glVertex3f(size, y, -size);
        glVertex3f(size, y, size);
        glVertex3f(-size, y, size);
        glEnd();
    }

    // Draw horizon mountains (voxel style)
    for (int i = 0; i < 20; i++) {
        float angle = (i / 20.0f) * 2.0f * 3.14159f;
        float x = cos(angle) * (SKYBOX_SIZE * 0.8f);
        float z = sin(angle) * (SKYBOX_SIZE * 0.8f);
        float height = 5.0f + (i % 8);

        Color mountainColor(0.4f, 0.3f, 0.5f);
        for (int h = 0; h < height; h++) {
            drawVoxelCube(x, h, z, 2.0f, mountainColor);
        }
    }

    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void Environment::drawOakTree(float x, float z, int height) {
    // Tree trunk - positioned so cubes are attached
    Color trunkColor(0.4f, 0.2f, 0.1f);
    for (int i = 0; i < height; i++) {
        drawVoxelCube(x, i * 0.6f + 0.3f, z, 0.6f, trunkColor); // Position cubes to touch
    }

    // Add connecting cubes between trunk and leaves
    float trunkTop = (height - 1) * 0.6f + 0.3f + 0.3f; // Top of last trunk cube
    int connectingCubes = 2; // Number of connecting cubes
    for (int i = 0; i < connectingCubes; i++) {
        float y = trunkTop + i * 0.6f + 0.3f;
        drawVoxelCube(x, y, z, 0.6f, trunkColor);
    }

    // Tree leaves (clustered voxels) - adjusted position
    Color leafColor(0.2f, 0.6f, 0.1f);
    float leavesStartY = trunkTop + connectingCubes * 0.6f;
    for (int dx = -2; dx <= 2; dx++) {
        for (int dz = -2; dz <= 2; dz++) {
            for (int dy = 0; dy < 3; dy++) {
                if (abs(dx) + abs(dz) + abs(dy) <= 3 && (dx + dz + dy + (int)x + (int)z) % 3 != 0) {
                    drawVoxelCube(x + dx, leavesStartY + dy * 0.8f, z + dz, 0.8f, leafColor);
                }
            }
        }
    }
}

void Environment::drawPineTree(float x, float z, int height) {
       // Tree trunk - positioned so cubes are attached
    Color trunkColor(0.3f, 0.2f, 0.1f);
    for (int i = 0; i < height; i++) {
        drawVoxelCube(x, i * 0.5f + 0.25f, z, 0.5f, trunkColor); // Position cubes to touch
    }

    // Add connecting cubes between trunk and leaves
    float trunkTop = (height - 1) * 0.5f + 0.25f + 0.25f; // Top of last trunk cube
    int connectingCubes = 1; // Number of connecting cubes
    for (int i = 0; i < connectingCubes; i++) {
        float y = trunkTop + i * 0.5f + 0.25f;
        drawVoxelCube(x, y, z, 0.5f, trunkColor);
    }

    // Pine leaves (triangular shape) - adjusted position
    Color leafColor(0.1f, 0.4f, 0.1f);
    float leavesStartY = trunkTop + connectingCubes * 0.5f;
    for (int layer = 0; layer < height - 1; layer++) {
        int radius = (height - layer) / 2;
        for (int dx = -radius; dx <= radius; dx++) {
            for (int dz = -radius; dz <= radius; dz++) {
                if (abs(dx) + abs(dz) <= radius && (dx + dz + layer) % 2 == 0) {
                    drawVoxelCube(x + dx * 0.7f, leavesStartY + layer * 0.6f, z + dz * 0.7f, 0.6f, leafColor);
                }
            }
        }
    }
}
void Environment::drawTrees() {
    setMaterial(Color(0.2f, 0.1f, 0.05f), Color(0.4f, 0.3f, 0.2f), Color(0.1f, 0.1f, 0.1f), 5.0f);

    for (std::vector<TreeData>::const_iterator it = trees.begin(); it != trees.end(); ++it) {
        const TreeData& tree = *it;
        if (tree.type == 0) {
            drawOakTree(tree.x, tree.z, static_cast<int>(tree.height));
        } else {
            drawPineTree(tree.x, tree.z, static_cast<int>(tree.height));
        }
    }
}

void Environment::drawGrass() {
    // Draw pre-generated grass patches
    setMaterial(Color(0.1f, 0.3f, 0.1f), Color(0.3f, 0.8f, 0.2f), Color(0.1f, 0.1f, 0.1f), 1.0f);

    for (std::vector<GrassData>::const_iterator it = grassPatches.begin(); it != grassPatches.end(); ++it) {
        const GrassData& grass = *it;

        // Add gentle swaying animation
        float sway = sin(time * 2.0f + grass.x * 0.1f) * 0.1f;

        glPushMatrix();
        glTranslatef(grass.x + sway, grass.height/2, grass.z);
        glRotatef(sway * 20.0f, 0, 0, 1); // Slight rotation for sway

        glColor3f(grass.color.r, grass.color.g, grass.color.b);
        glutSolidCube(grass.height);

        glPopMatrix();
    }
}

void Environment::drawClouds() {
    glPushMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Draw pre-generated clouds with gentle movement
    for (std::vector<CloudData>::const_iterator it = clouds.begin(); it != clouds.end(); ++it) {
        const CloudData& cloud = *it;

        // Gentle cloud movement
        float moveX = sin(time * 0.1f + cloud.x * 0.01f) * 2.0f;
        float moveY = sin(time * 0.15f + cloud.z * 0.01f) * 0.5f;

        Color cloudColor(0.95f + 0.05f * sin(time * 0.5f),
                        0.95f + 0.05f * sin(time * 0.7f),
                        1.0f);

        // Draw cloud as cluster of cubes
        for (int j = 0; j < 6; j++) {
            float offsetX = ((j % 3) - 1) * cloud.size;
            float offsetZ = ((j / 3) - 1) * cloud.size * 0.7f;
            float offsetY = (j % 2) * cloud.size * 0.3f;

            glColor3f(cloudColor.r, cloudColor.g, cloudColor.b);
            glPushMatrix();
            glTranslatef(cloud.x + moveX + offsetX, cloud.y + moveY + offsetY, cloud.z + offsetZ);
            glutSolidCube(cloud.size * 1.5f);
            glPopMatrix();
        }
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void Environment::drawParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (std::vector<Particle>::const_iterator it = particles.begin(); it != particles.end(); ++it) {
        const Particle& p = *it;
        if (p.life > 0.0f) {
            float alpha = p.life;
            glColor4f(p.color.r, p.color.g, p.color.b, alpha);

            glPushMatrix();
            glTranslatef(p.x, p.y, p.z);
            glutSolidSphere(0.1f, 4, 4);
            glPopMatrix();
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void Environment::updateParticles(float deltaTime) {
    for (std::vector<Particle>::iterator it = particles.begin(); it != particles.end(); ++it) {
        Particle& p = *it;
        if (p.life > 0.0f) {
            p.x += p.vx * deltaTime;
            p.y += p.vy * deltaTime;
            p.z += p.vz * deltaTime;
            p.vy -= 9.8f * deltaTime; // Gravity
            p.life -= deltaTime;
        }
    }
}

void Environment::addParticleEffect(float x, float y, float z, const Color& color) {
    for (int i = 0; i < 20; i++) {
        for (std::vector<Particle>::iterator it = particles.begin(); it != particles.end(); ++it) {
            Particle& p = *it;
            if (p.life <= 0.0f) {
                p.x = x;
                p.y = y;
                p.z = z;
                p.vx = ((rand() % 200) - 100) * 0.01f;
                p.vy = ((rand() % 100) + 50) * 0.02f;
                p.vz = ((rand() % 200) - 100) * 0.01f;
                p.life = 2.0f + (rand() % 100) * 0.01f;
                p.color = color;
                break;
            }
        }
    }
}

void Environment::addMagicEffect(float x, float z) {
    Color magicColor(0.8f + (rand() % 20) * 0.01f,
                    0.2f + (rand() % 40) * 0.01f,
                    0.9f + (rand() % 10) * 0.01f);
    addParticleEffect(x, 1.0f, z, magicColor);
}

void Environment::render() {
    // Draw environment components in proper order
    drawClouds();
    drawSkybox();
    drawFloor();
    drawBattleArena();
    drawGrass();
    drawTrees();
    drawParticles();

    // Add some ambient magical sparkles
    if ((int)(time * 2.0f) % 120 == 0) { // Every 2 seconds
        float x = (rand() % 20) - 10;
        float z = (rand() % 20) - 10;
        addMagicEffect(x, z);
    }
}

void Environment::update(float deltaTime) {
    time += deltaTime;
    updateParticles(deltaTime);
}

void Environment::toggleWireframe() {
    static bool wireframe = false;
    wireframe = !wireframe;
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Environment::cycleLighting() {
    currentLightMode = (currentLightMode + 1) % 4;

    switch (currentLightMode) {
        case 0: // Bright day mode
            {
                GLfloat ambient[] = {0.5f, 0.5f, 0.6f, 1.0f};
                GLfloat light0_diffuse[] = {1.0f, 1.0f, 0.9f, 1.0f};
                glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
            }
            break;
        case 1: // Golden sunset mode
            {
                GLfloat ambient[] = {0.6f, 0.4f, 0.3f, 1.0f};
                GLfloat light0_diffuse[] = {1.0f, 0.7f, 0.4f, 1.0f};
                glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
            }
            break;
        case 2: // Mystical twilight mode
            {
                GLfloat ambient[] = {0.3f, 0.2f, 0.5f, 1.0f};
                GLfloat light0_diffuse[] = {0.6f, 0.4f, 0.8f, 1.0f};
                glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
            }
            break;
        case 3: // Cool night mode
            {
                GLfloat ambient[] = {0.2f, 0.2f, 0.4f, 1.0f};
                GLfloat light0_diffuse[] = {0.4f, 0.5f, 0.8f, 1.0f};
                glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
                glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
            }
            break;
    }
}
