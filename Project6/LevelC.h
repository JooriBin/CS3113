#include "Scene.h"

class LevelC : public Scene {
public:
    // ————— STATIC ATTRIBUTES ————— //
    int PHASE2_COUNT = 11;
    int PHASE1_COUNT = 10;
    int KEYS_COUNT = 15;
    bool levels_passed = false;
    bool phase2_start = false;

    // ————— DESTRUCTOR ————— //
    ~LevelC();

    // ————— METHODS ————— //
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};

