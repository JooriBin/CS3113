#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 3
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5
#define FONTBANK_SIZE 16



#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"

// ————— GAME STATE ————— //
struct GameState
{
    Entity* player;
    Entity* enemies;

    Map* map;

    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
};

enum AppStatus { RUNNING, TERMINATED };

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;
//255, 204, 184
//247, 176, 205
//250, 202, 222
constexpr float BG_RED = 250/255.0f,
BG_BLUE = 222 / 255.0f,
BG_GREEN = 202 / 255.0f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char GAME_WINDOW_NAME[] = "Project 04!";

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/foxy_spritesheet.png",
MAP_TILESET_FILEPATH[] = "assets/images/tilemap_packed.png",
ENEMY_FILEPATH[] = "assets/images/enemy.png",
BGM_FILEPATH[] = "assets/audio/dooblydoo.wav",
JUMP_SFX_FILEPATH[] = "assets/audio/bounce.wav",
FONT_FILEPATH[] = "assets/font/font1.png";

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

unsigned int LEVEL_1_DATA[] =
{
    0, 41, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 41, 41, 0, 0, 0, 0, 0, 0, 0, 0,
    41, 41, 0, 0, 0, 0, 0, 0, 0, 41, 41, 41, 41, 41,
    17, 17, 41, 41, 0, 0, 41, 41, 41, 17, 17, 17, 17, 17,
    17, 17, 17, 17, 0, 0, 17, 17, 17, 17, 17, 17, 17, 17
};

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;
GLuint g_font_texture_id;
float g_previous_ticks = 0.0f,
g_accumulator = 0.0f;
bool m_game_active = true;


void initialise();
void process_input();
void update();
void render();
void shutdown();

// ————— GENERAL FUNCTIONS ————— //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return texture_id;
}

void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
        texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void initialise()
{
    // ————— GENERAL ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    // ————— VIDEO SETUP ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    glClear(GL_COLOR_BUFFER_BIT);

    // ————— MAP SET-UP ————— //
    GLuint map_texture_id = load_texture(MAP_TILESET_FILEPATH);
    g_game_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 15, 10);

    // ————— FOXY SET-UP ————— //

    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);

    int player_walking_animation[4][6] =
    {
        {7, 8, 9, 10, 11, 12},  // for foxy to move to the left,
        {7, 8, 9, 10, 11, 12}, // for foxy to move to the right,
        {48, 49, 50, 51, 52, 53 }, // for foxy to move upwards,
        {24, 25, 24, 25, 24, 25},   // for foxy to move downwards
    };

    //g_state.player->m_walking[g_state.player->LEFT] = new int[6] {48, 49, 50, 51, 52, 53};
    //g_state.player->m_walking[g_state.player->RIGHT] = new int[6] {48, 49, 50, 51, 52, 53};
    //g_state.player->m_walking[g_state.player->UP] = new int[6] { 48, 49, 50, 51, 52, 53 };
    //g_state.player->m_walking[g_state.player->DOWN] = new int[2] {24, 25};

    glm::vec3 acceleration = glm::vec3(0.0f, -4.905f, 0.0f);

    g_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        3.0f,                      // jumping power
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

    g_game_state.player->set_entity_type(PLAYER);

    //----- ENEMY SETUP ————— //
    GLuint enemy_texture_id = load_texture(ENEMY_FILEPATH);
    g_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        g_game_state.enemies[i] = Entity(enemy_texture_id, 0.5f, 0.5f, 0.5f, ENEMY, GUARD, IDLE);
    }

    //repat this process for the other two
    g_game_state.enemies[0].set_position(glm::vec3(3.0f, 0.0f, 0.0f));
    g_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    g_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    g_game_state.enemies[0].set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    g_game_state.enemies[0].set_jumping_power(5.5f);


    g_game_state.enemies[1].set_position(glm::vec3(10.0f, 0.0f, 0.0f));
    g_game_state.enemies[1].set_movement(glm::vec3(0.0f));
    g_game_state.enemies[1].set_acceleration(glm::vec3(0.0f, -5.0f, 0.0f));
    g_game_state.enemies[1].set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    g_game_state.enemies[1].set_ai_type(JUMPER);
    g_game_state.enemies[1].set_jumping_power(3.0f);

    g_game_state.enemies[2].set_position(glm::vec3(3.75f, 0.0f, 0.0f));
    g_game_state.enemies[2].set_movement(glm::vec3(0.0f));
    g_game_state.enemies[2].set_acceleration(glm::vec3(0.0f, -4.0f, 0.0f));
    g_game_state.enemies[2].set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    g_game_state.enemies[2].set_ai_type(WALKER);
    g_game_state.enemies[2].set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    g_game_state.enemies[2].set_init_pos(4.0f);
    g_game_state.enemies[2].set_jumping_power(1.0f);




    // Jumping
    g_game_state.player->set_jumping_power(5.0f);

    //font 
    g_font_texture_id = load_texture(FONT_FILEPATH);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    g_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    //    Mix_PlayMusic(g_game_state.bgm, -1);
    //    Mix_VolumeMusic(MIX_MAX_VOLUME / 16.0f);

    g_game_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);

    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_app_status = TERMINATED;
                break;

            case SDLK_SPACE:
                // Jump
                if (g_game_state.player->get_collided_bottom())
                {
                    g_game_state.player->jump();
                    Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT]) {
        g_game_state.player->move_left();
    }
    else if (key_state[SDL_SCANCODE_RIGHT]) {
        g_game_state.player->move_right();
    }
    if (key_state[SDL_SCANCODE_UP]) g_game_state.player->move_up();
    else if(key_state[SDL_SCANCODE_DOWN]) g_game_state.player->move_down();

    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
        g_game_state.player->normalise_movement();
}

void update()
{
        float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;

        delta_time += g_accumulator;

        if (delta_time < FIXED_TIMESTEP)
        {
            g_accumulator = delta_time;
            return;
        }

        while (delta_time >= FIXED_TIMESTEP)
        {
            for (int i = 0; i < ENEMY_COUNT; i++) {
                g_game_state.enemies[i].update(FIXED_TIMESTEP, g_game_state.player, NULL, 0, g_game_state.map);
            }

            g_game_state.player->update(FIXED_TIMESTEP,
                g_game_state.player, NULL, 0, g_game_state.map);
            delta_time -= FIXED_TIMESTEP;
        }

        g_accumulator = delta_time;

        g_view_matrix = glm::mat4(1.0f);
        int dis_enemy = 0;
        for (int i = 0; i < ENEMY_COUNT; i++) {
            if (!g_game_state.enemies[i].is_active())
                dis_enemy++;
        }

        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_game_state.player->get_position().x, 0.0f, 0.0f));


}

void render()
{   
    g_shader_program.set_view_matrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!g_game_state.player->is_active()) {
        draw_text(&g_shader_program, g_font_texture_id, "Y", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) - 1.5f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "O", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) - 1.0f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "U", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) - 0.5f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, " ", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x), 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "L", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) + 0.5f , 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "O", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) +1.0f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "S", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) + 1.5f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "E", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) + 2.0f, 2.0f, 0.0f));
    }
    int dis_enemy = 0;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (!g_game_state.enemies[i].is_active())
            dis_enemy++;
    }
    if (dis_enemy == 3) {
        draw_text(&g_shader_program, g_font_texture_id, "Y", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) - 1.5f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "O", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) - 1.0f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "U", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) - 0.5f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, " ", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x), 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "W", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) + 0.5f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "I", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) + 1.0f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "N", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) + 1.5f, 2.0f, 0.0f));
        draw_text(&g_shader_program, g_font_texture_id, "!", 0.5f, 0.05f,
            glm::vec3((g_game_state.player->get_position().x) + 2.0f, 2.0f, 0.0f));
    }

    g_game_state.map->render(&g_shader_program);

   
    g_game_state.player->render(&g_shader_program);
    for (int i = 0; i < ENEMY_COUNT; i++) {
        g_game_state.enemies[i].render(&g_shader_program);
    }

    



    SDL_GL_SwapWindow(g_display_window);






}

void shutdown()
{
    SDL_Quit();

    delete[] g_game_state.enemies;
    delete    g_game_state.player;
    delete    g_game_state.map;
    Mix_FreeChunk(g_game_state.jump_sfx);
    Mix_FreeMusic(g_game_state.bgm);
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}