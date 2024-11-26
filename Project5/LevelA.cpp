#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 17
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/foxy_spritesheet.png",
           PLATFORM_FILEPATH[]    = "assets/tile0041.png",
           ENEMY_FILEPATH[]       = "assets/enemy.png";
GLuint g_font_texture_id0;

//MUSIC
//constexpr int CD_QUAL_FREQ = 44100,  // compact disk (CD) quality frequency
//AUDIO_CHAN_AMT = 2,
//AUDIO_BUFF_SIZE = 4096;
//constexpr char BGM_FILEPATH[] = "music.mp3";
//constexpr int    LOOP_FOREVER = -1;
Mix_Music* g_music;

unsigned int LEVEL_DATA[] =
{
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 103, 104,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 118, 119,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 41, 41, 41, 41, 41, 0, 34, 34,
    2, 41, 41, 41, 41, 41, 41, 41, 41, 17, 17, 17, 17, 17, 0, 0,0,
    2, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 0, 0,0
};

LevelA::~LevelA()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise()
{

    GLuint map_texture_id = Utility::load_texture("assets/tilemap_packed.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 15, 10);
    g_font_texture_id0 = Utility::load_texture("assets/font1.png");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    // Start Audio
    //Mix_OpenAudio(
    //    CD_QUAL_FREQ,        // the frequency to playback audio at (in Hz)
    //    MIX_DEFAULT_FORMAT,  // audio format
    //    AUDIO_CHAN_AMT,      // number of channels (1 is mono, 2 is stereo, etc).
    //    AUDIO_BUFF_SIZE      // audio buffer size in sample FRAMES (total samples divided by channel count)
    //);
    //g_music = Mix_LoadMUS(BGM_FILEPATH);

    //// This will schedule the music object to begin mixing for playback.
    //// The first parameter is the pointer to the mp3 we loaded 
    //// and second parameter is the number of times to loop.
    //Mix_PlayMusic(g_music, LOOP_FOREVER);

    //// Set the music to half volume
    //// MIX_MAX_VOLUME is a pre-defined constant
    //Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

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
    
    m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));

    
    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
    m_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 0.5f, 0.5f, ENEMY, GUARD, IDLE);
    }


    m_game_state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
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

void LevelA::update(float delta_time)
{
    if (!m_game_state.player->is_active() && player_lives > 0) {
        player_lives--;
        m_game_state.player->activate();
    }
    if(m_game_state.player->get_position().y < -10.0f) {
        player_lives--;
        m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));

    }
    m_game_state.player->update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    
    for (int i = 0; i < ENEMY_COUNT; i++)
    {

        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);

    }

    if (m_game_state.player->get_position().x == 1.0f) {
        if (m_game_state.player->get_position().x > 15.0f) m_game_state.next_scene_id = 2;
    }
}


void LevelA::render(ShaderProgram *g_shader_program)
{
    Utility::draw_text(g_shader_program, g_font_texture_id0, "Land on the door", 0.4f, 0.05f,
        glm::vec3(2.0f, -1.0f, 0.0f));
    Utility::draw_text(g_shader_program, g_font_texture_id0, " at the other side", 0.4f, 0.05f,
        glm::vec3(2.0f, -2.0f, 0.0f));
    m_game_state.map->render(g_shader_program);
    if (player_lives > 0) {
        m_game_state.player->render(g_shader_program);

    }
    for (int i = 0; i < m_number_of_enemies; i++) m_game_state.enemies[i].render(g_shader_program);
}
