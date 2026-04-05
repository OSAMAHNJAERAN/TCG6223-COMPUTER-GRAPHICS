#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <GL/glut.h>
#include <vector>
#include <cmath>

class Environment {
private:
    // Environment dimensions
    static const int FLOOR_SIZE = 20;
    static const int SKYBOX_SIZE = 50;

    // Texture management
    struct Color {
        float r, g, b;
        Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f) : r(red), g(green), b(blue) {}
    };

    // Tree structure for procedural generation
    struct TreeData {
        float x, z;
        float height;
        int type; // 0 = oak, 1 = pine
        TreeData(float px, float pz, float h, int t) : x(px), z(pz), height(h), type(t) {}
    };

    // Grass patch structure
    struct GrassData {
        float x, z, height;
        Color color;
        GrassData(float px, float pz, float h, Color c) : x(px), z(pz), height(h), color(c) {}
    };

    // Cloud structure
    struct CloudData {
        float x, y, z;
        float size;
        CloudData(float px, float py, float pz, float s) : x(px), y(py), z(pz), size(s) {}
    };

    // Fixed data arrays (generated once)
    std::vector<TreeData> trees;
    std::vector<GrassData> grassPatches;
    std::vector<CloudData> clouds;
    std::vector<std::vector<Color> > floorColors; // 2D array for floor
    bool initialized;

    // Animation variables
    float time;
    float lightIntensity;
    int currentLightMode;

    // Particle system for effects
    struct Particle {
        float x, y, z;
        float vx, vy, vz;
        float life;
        Color color;
        Particle() : x(0), y(0), z(0), vx(0), vy(0), vz(0), life(0) {}
    };
    std::vector<Particle> particles;

    // Lighting setup
    void setupLighting();
    void updateLighting();

    // Drawing functions
    void drawVoxelCube(float x, float y, float z, float size, const Color& color);
    void drawTexturedQuad(float x1, float y1, float z1, float x2, float y2, float z2,
                         float x3, float y3, float z3, float x4, float y4, float z4, const Color& color);
    void drawFloor();
    void drawSkybox();
    void drawTrees();
    void drawOakTree(float x, float z, int height);
    void drawPineTree(float x, float z, int height);
    void drawGrass();
    void drawClouds();
    void drawParticles();
    void drawBattleArena();
    void drawMountains();



    // Utility functions
    void setMaterial(const Color& ambient, const Color& diffuse, const Color& specular, float shininess);
    void generateStaticContent(); // Generate all static content once
    void updateParticles(float deltaTime);
    void addParticleEffect(float x, float y, float z, const Color& color);

public:
    Environment();
    ~Environment();

    void initialize();
    void render();
    void update(float deltaTime);

    // Environment controls
    void toggleWireframe();
    void cycleLighting();
    void addMagicEffect(float x, float z); // For character skills later

    // Getters for environment boundaries (useful for character placement later)
    float getFloorSize() const { return FLOOR_SIZE; }
    float getFloorHeight() const { return 0.0f; }
};

#endif // ENVIRONMENT_HPP
