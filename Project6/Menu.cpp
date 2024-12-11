#include "Menu.h"
#include "Utility.h"

#define LEVEL_WIDTH 3
#define LEVEL_HEIGHT 3

constexpr char SPRITESHEET_FILEPATH[] = "assets/foxy_spritesheet.png",
PLATFORM_FILEPATH[] = "assets/tile0041.png",
FONT_FILEPATH[]="assets/font1.png",
ENEMY_FILEPATH[] = "assets/enemy.png";
GLuint g_font_texture_id;

unsigned int MENU_DATA[] =
{
    0,0,0,
    0,0,0,
    0,0,0
};

Menu::~Menu()
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

void Menu::initialise()
{
    GLuint map_texture_id = Utility::load_texture("assets/tilemap_packed.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, MENU_DATA, map_texture_id, 1.0f, 15, 10);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    g_font_texture_id = Utility::load_texture("assets/font1.png");
    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    int player_walking_animation[4][6] =
    {
        {7, 8, 9, 10, 11, 12},  // for foxy to move to the left,
        {7, 8, 9, 10, 11, 12}, // for foxy to move to the right,
        {48, 49, 50, 51, 52, 53 }, // for foxy to move upwards,
        {24, 25, 24, 25, 24, 25},   // for foxy to move downwards
    };
    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
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


    
    //Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    //m_game_state.bgm = Mix_LoadMUS("assets/Pookatori and Friends.mp3");
    //Mix_PlayMusic(m_game_state.bgm, -1);
    //Mix_VolumeMusic(50.0f);

}

void Menu::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    if (next_level == true) {
        m_game_state.next_scene_id = 1;
    }
}


void Menu::render(ShaderProgram* g_shader_program)
{
    Utility::draw_text(g_shader_program, g_font_texture_id, "FOXY FIGHT!", 0.5f, 0.05f,
        glm::vec3(-2.75f, 1.0f, 0.0f));
    Utility::draw_text(g_shader_program, g_font_texture_id, "Press ENTER to start", 0.35f, 0.001f,
        glm::vec3(-3.5f, 0.0f, 0.0f));
    m_game_state.map->render(g_shader_program);  

}
