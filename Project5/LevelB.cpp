#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/foxy_spritesheet.png",
PLATFORM_FILEPATH[] = "assets/tile0041.png",
FONT_FILEPATH[] = "assets/font1.png",
ENEMY_FILEPATH[] = "assets/enemy.png";

GLuint g_font_texture_id2;


unsigned int LEVELB_DATA[] =
{
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41, 2, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 41, 2, 2, 2, 2,
    2, 41, 41, 41, 41, 41, 41, 0, 41, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2
};

LevelB::~LevelB()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::initialise()
{
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

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        3.0f,                      // speed
        acceleration,              // acceleration
        4.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        6,                         // animation column amount
        12,                         // animation row amount
        0.9f,                      // width
        0.9f,                       // height
        PLAYER
    );

    m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));


    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 0.5f, 0.5f, ENEMY, WALKER, IDLE);
    }


    m_game_state.enemies[0].set_position(glm::vec3(4.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_game_state.enemies[0].set_scale(glm::vec3(0.5f, 0.5f, 0.0f));


    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/Pookatori and Friends.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(100.0f);

    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelB::update(float delta_time)
{
    if (!m_game_state.player->is_active() && player_lives > 0) {
        player_lives--;
        m_game_state.player->activate();
    }

    m_game_state.player->update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    for (int i = 0; i < ENEMY_COUNT; i++)
    {

        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

    }
    if (m_game_state.player->get_position().x == 1.0f) {
        if (m_game_state.player->get_position().y < -10.0f && !m_game_state.enemies[0].is_active() && m_game_state.player->get_position().x > 10.0f) m_game_state.next_scene_id = 3;
    }
    else {
        if (m_game_state.player->get_position().y < -10.0f && (m_game_state.enemies[0].is_active()|| m_game_state.player->get_position().x <= 10.0f)) {
            player_lives--;
            m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));

        }
    }
}


void LevelB::render(ShaderProgram* g_shader_program)
{
        if (m_game_state.enemies[0].is_active() && player_lives > 0) {
            Utility::draw_text(g_shader_program, g_font_texture_id2, "Kill the enemy!", 0.35f, 0.05f,
                glm::vec3(2.0f, -1.0f, 0.0f));
            Utility::draw_text(g_shader_program, g_font_texture_id2, "then jump on ", 0.35f, 0.05f,
                glm::vec3(2.0f, -2.0f, 0.0f));
            Utility::draw_text(g_shader_program, g_font_texture_id2, "the other end", 0.35f, 0.05f,
                glm::vec3(2.0f, -3.0f, 0.0f));
    }

    m_game_state.map->render(g_shader_program);
    if (player_lives > 0) {
        m_game_state.player->render(g_shader_program);

    }
    for (int i = 0; i < m_number_of_enemies; i++)
        m_game_state.enemies[i].render(g_shader_program);
}
