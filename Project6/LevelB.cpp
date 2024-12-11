#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 11
#define LEVEL_HEIGHT 9

constexpr char SPRITESHEET_FILEPATH[] = "assets/foxy_spritesheet.png",
PLATFORM_FILEPATH[] = "assets/tile0041.png",
FONT_FILEPATH[] = "assets/font1.png",
ENEMY_FILEPATH[] = "assets/enemy.png",
SHOOT_SFX_FILEPATH[] = "assets/pow.mp3";


GLuint g_font_texture_id2;


unsigned int LEVELB_DATA[] =
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

LevelB::~LevelB()
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

void LevelB::initialise()
{
    m_number_of_enemies = ENEMY_COUNT;
    GLuint map_texture_id = Utility::load_texture("assets/tilemap_packed.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 15, 10);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    g_font_texture_id2 = Utility::load_texture("assets/font1.png");

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
        2.0f,                      // speed
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
    m_game_state.player->set_position(glm::vec3(5.0f, -7.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    m_game_state.player->set_bullet_texture(bullet_texture);

    GLuint bomb_texture = Utility::load_texture("assets/exp.png");
    m_game_state.player->set_bomb_texture(bomb_texture);


    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    GLuint Ebullet_texture = Utility::load_texture("assets/Ebullet.png");
    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT/2 ; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 0.25f, 0.25f, ENEMY, WALKER, IDLE);
        m_game_state.enemies[i].set_bullet_texture(Ebullet_texture);
        //m_game_state.enemies[i].set_shoot_interval(5.0f);
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
        m_game_state.enemies[i].set_movement(glm::vec3(0.5f, 0.0f,0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        m_game_state.enemies[i].set_scale(glm::vec3(0.25f, 0.25f, 0.0f));
        m_game_state.enemies[i].set_shoot_interval(100);

    }

    for (int i = ENEMY_COUNT / 2; i < ENEMY_COUNT ; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_texture_id, 0.35f, 0.25f, 0.25f, ENEMY, GUARD, IDLE);

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

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        3.0f,                      // speed
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
    m_game_state.player->set_position(glm::vec3(5.0f, -7.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    m_game_state.player->set_bullet_texture(bullet_texture);
    m_game_state.player->set_bomb_texture(bomb_texture);


    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 5096);

    m_game_state.bgm = Mix_LoadMUS("assets/Pookatori and Friends.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(50.0f);

    m_game_state.shoot_sfx = Mix_LoadWAV("assets/pow.wav");
    m_game_state.bomb_sfx = Mix_LoadWAV("assets/boom.wav");
    m_game_state.die_sfx = Mix_LoadWAV("assets/die.wav");

}

void LevelB::update(float delta_time)
{
    if (!m_game_state.player->is_active() && player_lives > 0) {
        player_lives--;
        m_game_state.player->activate();
                m_game_state.player->set_position(glm::vec3(5.0f, -7.0f, 0.0f));

    }

    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    for (int i = 0; i < ENEMY_COUNT; i++)
    {

        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

    }
    bool next_scene = true;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (m_game_state.enemies[i].is_active()) {
            next_scene = false;
        }
    }
    if (next_scene && player_lives != 0) {
        next_level = true;
        m_game_state.next_scene_id = 3;
    }
}


void LevelB::render(ShaderProgram* g_shader_program)
{


    m_game_state.map->render(g_shader_program);
    if (player_lives > 0) {
        m_game_state.player->render(g_shader_program);

    }
    for (int i = 0; i < ENEMY_COUNT; i++)
        m_game_state.enemies[i].render(g_shader_program);
}
