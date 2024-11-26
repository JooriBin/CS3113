#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 18
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/foxy_spritesheet.png",
PLATFORM_FILEPATH[] = "assets/tile0041.png",
FONT_FILEPATH[] = "assets/font1.png",
ENEMY_FILEPATH[] = "assets/enemy.png";

GLuint g_font_texture_id4;


unsigned int LEVELC_DATA[] =
{
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41, 41, 41, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 17, 17, 0, 15, 0,
    2, 0, 41, 41, 41, 41, 41, 0, 0, 0, 0, 41, 17, 17, 17, 0, 0, 0,
    2, 41, 17, 17, 17, 17, 17, 41, 41, 41, 41, 17, 17, 17, 17, 0, 0, 0,
    2, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 0, 0, 0
};

LevelC::~LevelC()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelC::initialise()
{
    GLuint map_texture_id = Utility::load_texture("assets/tilemap_packed.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 15, 10);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    g_font_texture_id4 = Utility::load_texture("assets/font1.png");

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
        5.0f,                      // jumping power
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
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 0.5f, 0.5f, ENEMY, JUMPER, IDLE);
    }


    m_game_state.enemies[0].set_position(glm::vec3(10.0f, -2.0f, 0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_game_state.enemies[0].set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    m_game_state.enemies[0].set_jumping_power(7.0f);




    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_game_state.bgm = Mix_LoadMUS("assets/Pookatori and Friends.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(100.0f);

    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelC::update(float delta_time)
{
    if (!m_game_state.player->is_active() && player_lives > 0) {
        player_lives--;
        m_game_state.player->activate();
    }
    if (m_game_state.player->get_position().y < -10.0f) {
        player_lives--;
        m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));

    }
    m_game_state.player->update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    for (int i = 0; i < ENEMY_COUNT; i++)
    {

        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

    }
    if (m_game_state.player->get_position().x == 16.0f) {
        m_game_state.next_scene_id = 3;
        m_game_state.player->levels_passed = true;
    }
    
}


void LevelC::render(ShaderProgram* g_shader_program)
{
    if (m_game_state.player->levels_passed) {
        Utility::draw_text(g_shader_program, g_font_texture_id4, "YOU WON!", 0.4f, 0.05f,
            glm::vec3(m_game_state.player->get_position().x -1.0f, m_game_state.player->get_position().y + 2.5f, 0.0f));
    }
    else {
        Utility::draw_text(g_shader_program, g_font_texture_id4, "Land on the platform", 0.4f, 0.05f,
            glm::vec3(2.0f, -1.0f, 0.0f));
        Utility::draw_text(g_shader_program, g_font_texture_id4, " at the end of the map", 0.4f, 0.05f,
            glm::vec3(2.0f, -2.0f, 0.0f));
    }
  

    m_game_state.map->render(g_shader_program);
    if (player_lives > 0) {
        m_game_state.player->render(g_shader_program);

    }
    for (int i = 0; i < m_number_of_enemies; i++)
        m_game_state.enemies[i].render(g_shader_program);
}
