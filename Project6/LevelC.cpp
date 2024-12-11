#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 11
#define LEVEL_HEIGHT 9

constexpr char SPRITESHEET_FILEPATH[] = "assets/foxy_spritesheet.png",
PLATFORM_FILEPATH[] = "assets/tile0041.png",
FONT_FILEPATH[] = "assets/font1.png",
ENEMY_FILEPATH[] = "assets/flower.png",
KEY_FILEPATH[] = "assets/keyy.png",
SHOOT_SFX_FILEPATH[] = "assets/pow.mp3";

GLuint g_font_texture_id4;


unsigned int LEVELC_DATA[] =
{
  2,2,2,2,2,2,2,2,2,2,2,
  2,11,11,11,11,11,11,11,11,11,2,
  2,0,0,0,0,0,0,0,0,0,2,
  2,0,0,0,0,0,0,0,0,0,2,
  2,0,0,0,0,0,0,0,0,0,2,
  2,0,0,0,0,0,0,0,0,0,2,
  2,0,0,0,0,0,0,0,0,0,2,
  2,0,0,0,0,0,0,0,0,0,2,
  2,41,41,41,41,41,41,41,41,41,2
};

LevelC::~LevelC()
{
    delete[] m_game_state.enemies;
    delete[] m_game_state.keys;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.shoot_sfx);
    Mix_FreeChunk(m_game_state.die_sfx);
    Mix_FreeChunk(m_game_state.bomb_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelC::initialise()
{
    m_number_of_enemies = PHASE2_COUNT + PHASE1_COUNT;
    GLuint map_texture_id = Utility::load_texture("assets/tilemap_packed.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 15, 10);


    //PLAYER
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    g_font_texture_id4 = Utility::load_texture("assets/font1.png");

    int player_walking_animation[4][6] =
    {
        {7, 8, 9, 10, 11, 12},  // for foxy to move to the left,
        {7, 8, 9, 10, 11, 12}, // for foxy to move to the right,
        {48, 49, 50, 51, 52, 53 }, // for foxy to move upwards,
        {24, 25, 24, 25, 24, 25},   // for foxy to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        2.2f,                      // speed
        acceleration,              // acceleration
        0.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        6,                         // animation column amount
        12,                         // animation row amount
        0.45f,                      // width
        0.45f,                       // height
        PLAYER
    );
    GLuint bullet_texture = Utility::load_texture("assets/Bullet.png");
    m_game_state.player->set_position(glm::vec3(5.0f, -6.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    m_game_state.player->set_bullet_texture(bullet_texture);

    GLuint bomb_texture = Utility::load_texture("assets/exp.png");
    m_game_state.player->set_bomb_texture(bomb_texture);


    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    GLuint Ebullet_texture = Utility::load_texture("assets/Ebullet.png");
    m_game_state.enemies = new Entity[(PHASE2_COUNT + PHASE1_COUNT)];


    for (int i = 0; i < PHASE1_COUNT / 2; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 0.25f, 0.25f, ENEMY, SHOOTER, IDLE);
        m_game_state.enemies[i].set_bullet_texture(Ebullet_texture);



        float lower_x = 1.0f;  // Lower limit
        float upper_x = 9.0f; // Upper limit

        float rand_x = 5.0f;
        float rand_y = -7.0f;
        bool position_valid;


        do {
            rand_x = lower_x + (static_cast<float>(std::rand()) / RAND_MAX) * (upper_x - lower_x);
            position_valid = true;
            for (int j = 0; j < i; j++) { // Check against previously placed enemies
                float distance_x = fabs(rand_x - m_game_state.enemies[j].get_position().x);
                if (distance_x < 0.5f) { // Adjust the threshold as needed
                    position_valid = false;
                    break;
                }
            }
        } while (!position_valid);

        m_game_state.enemies[i].set_position(glm::vec3(rand_x, -7.30f, 0.0f));
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.enemies[i].set_scale(glm::vec3(0.25f, 0.25f, 0.0f));
        m_game_state.enemies[i].set_shoot_interval(40);
    }

    enemy_texture_id = Utility::load_texture("assets/enemy.png");

    for (int i = PHASE1_COUNT / 2; i < PHASE1_COUNT; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_texture_id, 0.175f, 0.25f, 0.25f, ENEMY, GUARD, IDLE);

        float lower_y = -4.0f;  // Lower limit
        float upper_y = -2.0f; // Upper limit

        float lower_x = 1.0f;  // Lower limit
        float upper_x = 9.0f; // Upper limit

        float rand_x = 5.0f;
        float rand_y = -7.0f;
        bool position_valid;

        do {
            rand_x = lower_x + (static_cast<float>(std::rand()) / RAND_MAX) * (upper_x - lower_x);
            rand_y = lower_y + (static_cast<float>(std::rand()) / RAND_MAX) * (upper_y - lower_y);

            position_valid = true;
            for (int j = 0; j < i; j++) { // Check against previously placed enemies
                float distance_x = fabs(rand_x - m_game_state.enemies[j].get_position().x);
                float distance_y = fabs(rand_y - m_game_state.enemies[j].get_position().y);
                if (distance_x < 0.5f && distance_y < 0.5f) { // Adjust the threshold as needed
                    position_valid = false;
                    break;
                }
            }
        } while (!position_valid);

        m_game_state.enemies[i].set_position(glm::vec3(rand_x, rand_y, 0.0f));
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.enemies[i].set_scale(glm::vec3(0.25f, 0.25f, 0.0f));
    }


    enemy_texture_id = Utility::load_texture("assets/enemy2.png");
    //for (int i = 0; i < PHASE1_COUNT; i++)
    //{
    //    m_game_state.enemies[i].deactivate();
    //}
        m_game_state.enemies[PHASE1_COUNT] = Entity(enemy_texture_id, 0.5f, 2.0f, 2.0f, ENEMY, FINALBOSS, IDLE);
        m_game_state.enemies[PHASE1_COUNT].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[PHASE1_COUNT].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.enemies[PHASE1_COUNT].set_scale(glm::vec3(2.0f, 2.0f, 0.0f));
        m_game_state.enemies[PHASE1_COUNT].set_position(glm::vec3(5.0f, -3.0f, 0.0f));
        m_game_state.enemies[PHASE1_COUNT].deactivate();

 
    for (int i = PHASE1_COUNT + 1; i < (PHASE2_COUNT + PHASE1_COUNT); i++)
    {

        m_game_state.enemies[i] = Entity(enemy_texture_id, 0.25f, 0.25f, 0.25f, ENEMY, GUARD, IDLE);
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.enemies[i].set_scale(glm::vec3(0.25f, 0.25f, 0.0f));
        m_game_state.enemies[i].deactivate();


        float lower_y = -5.0f;  // Lower limit
        float upper_y = -2.0f; // Upper limit
        float lower_x = 1.0f;  // Lower limit
        float upper_x = 9.0f; // Upper limit

        float rand_x = 5.0f;
        float rand_y = -7.0f;
        bool position_valid;

        do {
            rand_x = lower_x + (static_cast<float>(std::rand()) / RAND_MAX) * (upper_x - lower_x);
            rand_y = lower_y + (static_cast<float>(std::rand()) / RAND_MAX) * (upper_y - lower_y);

            position_valid = true;
            for (int j = 0; j < i; j++) { // Check against previously placed enemies
                float distance_x = fabs(rand_x - m_game_state.enemies[j].get_position().x);
                float distance_y = fabs(rand_y - m_game_state.enemies[j].get_position().y);
                if (distance_x < 0.5f && distance_y < 0.5f) { // Adjust the threshold as needed
                    position_valid = false;
                    break;
                }
            }
        } while (!position_valid);

        m_game_state.enemies[i].set_position(glm::vec3(rand_x, rand_y, 0.0f));
    }


    //KEYS
    GLuint keys_texture_id = Utility::load_texture(KEY_FILEPATH);
    m_game_state.keys = new Entity[KEYS_COUNT];

    for (int i = 0; i < KEYS_COUNT; i++) {
        m_game_state.keys[i] = Entity();
        m_game_state.keys[i].set_entity_type(KEYS);
        m_game_state.keys[i].set_texture_id(keys_texture_id); // Ensure texture is set
        m_game_state.keys[i].activate(); // Activate the key
        float lower_y = -4.0f;  // Lower limit
        float upper_y = -2.0f; // Upper limit

        float lower_x = 1.0f;  // Lower limit
        float upper_x = 9.0f; // Upper limit

        float rand_x = 5.0f;
        float rand_y = -7.0f;
        bool position_valid;


        do {
            rand_x = lower_x + (static_cast<float>(std::rand()) / RAND_MAX) * (upper_x - lower_x);
            rand_y = lower_y + (static_cast<float>(std::rand()) / RAND_MAX) * (upper_y - lower_y);

            position_valid = true;
            for (int j = 0; j < i; j++) { // Check against previously placed enemies
                float distance_x = fabs(rand_x - m_game_state.keys[j].get_position().x);
                float distance_y = fabs(rand_y - m_game_state.keys[j].get_position().y);
                if (distance_x < 0.5f && distance_y < 0.5f) { // Adjust the threshold as needed
                    position_valid = false;
                    break;
                }
            }
        } while (!position_valid);

        m_game_state.keys[i].set_position(glm::vec3(rand_x, rand_y, 0.0f));
        m_game_state.keys[i].set_width(0.25f);
        m_game_state.keys[i].set_height(0.25f);
        m_game_state.keys[i].set_movement(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.keys[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.keys[i].set_scale(glm::vec3(0.25f, 0.25f, 0.0f));
    }






        /**
         BGM and SFX
         */
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 6096);

        m_game_state.bgm = Mix_LoadMUS("assets/Pookatori and Friends.mp3");
        Mix_PlayMusic(m_game_state.bgm, -1);
        Mix_VolumeMusic(50.0f);

        m_game_state.shoot_sfx = Mix_LoadWAV("assets/pow.wav");
        m_game_state.bomb_sfx = Mix_LoadWAV("assets/boom.wav");
        m_game_state.die_sfx = Mix_LoadWAV("assets/die.wav");
        m_game_state.bomb_sfx = Mix_LoadWAV("assets/boom.wav");
        m_game_state.collect_sfx = Mix_LoadWAV("assets/keysound.wav");
    
}

void LevelC::update(float delta_time)
{

    if (m_game_state.player->m_collected_keys == KEYS_COUNT && !phase2_start) {
        for(int i = PHASE1_COUNT; i < (PHASE2_COUNT + PHASE1_COUNT); i++) {
            m_game_state.enemies[i].activate();
        }
        phase2_start = true;
    }
    if (!m_game_state.player->is_active() && player_lives > 0) {
        player_lives--;
        m_game_state.player->activate();
        m_game_state.player->set_position(glm::vec3(5.0f, -6.0f, 0.0f));

    }

    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, (PHASE1_COUNT+PHASE2_COUNT), m_game_state.map);
    
    if (m_game_state.player->m_collected_keys != KEYS_COUNT) {

        for (int i = 0; i < PHASE1_COUNT; i++)
        {
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
        }
    }
    else {
        for (int i = 0; i < PHASE1_COUNT; i++)
        {
            m_game_state.enemies[i].deactivate();
        }
        for (int i = PHASE1_COUNT; i < PHASE1_COUNT + PHASE2_COUNT; i++)
       {
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
        }
    }
    for (int i = 0; i < KEYS_COUNT; i++)
    {
        m_game_state.keys[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
        

    }
    //winnnig condition (moves to next scene)
    bool next_scene = true;
    for (int i = 0; i < PHASE2_COUNT + PHASE1_COUNT; i++) {
        if (m_game_state.enemies[i].is_active()) {
            next_scene = false;
        }
    }
    if (next_scene && player_lives != 0) {
        m_game_state.player->levels_passed = true;
    }
}


void LevelC::render(ShaderProgram* g_shader_program)
{
    if (m_game_state.player->levels_passed) {
        Utility::draw_text(g_shader_program, g_font_texture_id4, "YOU WON!", 0.4f, 0.05f,
            glm::vec3(3.0f, -4.0f, 0.0f));
    }

    if ((m_game_state.enemies[PHASE1_COUNT].is_active())) {
        std::string boss_health_str = std::to_string((int)m_game_state.enemies[PHASE1_COUNT].boss_health);
        Utility::draw_text(g_shader_program, g_font_texture_id4, boss_health_str, 0.2f, 0.00001f,
            glm::vec3(m_game_state.enemies[PHASE1_COUNT].get_position().x, m_game_state.enemies[PHASE1_COUNT].get_position().y + 1.25f, 0.0f));
    }
    

 
    m_game_state.map->render(g_shader_program);
    if (player_lives > 0) {
        m_game_state.player->render(g_shader_program);

    }

    if (m_game_state.player->m_collected_keys != KEYS_COUNT) {

        for (int i = 0; i < PHASE1_COUNT; i++)
        {
            m_game_state.enemies[i].render(g_shader_program);
        }
    }
    else {
        for (int i = PHASE1_COUNT; i < (PHASE1_COUNT + PHASE2_COUNT); i++)
        {
            m_game_state.enemies[i].render(g_shader_program);
        }
    }

    for (int i = 0; i < KEYS_COUNT; i++)
    {
        m_game_state.keys[i].render(g_shader_program);
    }
}
