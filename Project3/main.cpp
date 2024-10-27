/**
* Author: Joori Bin Jardan
* Assignment: Lunar Lander
* Date due: 2024-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define GREEN_PLATFORM_COUNT 5
#define RED_PLATFORM_COUNT 8


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

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


//delete

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* green_platforms;
    Entity* red_platforms;
    Entity* background;
};

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;
constexpr int FONTBANK_SIZE = 16;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;
const char SPRITESHEET_FILEPATH[] = "assets/foxy_spritesheet.png";
const char GREEN_PLATFORM_FILEPATH[] = "assets/tile_0018.png"; 
const char FONT_FILEPATH[] = "assets/font1.png";
const char RED_PLATFORM_FILEPATH[] = "assets/tile_0013.png";
const char BACKGROUND_FILEPATH[] = "assets/backgroundEmpty.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

// ––––– GLOBAL VARIABLES ––––– //
GameState g_state;

GLuint g_font_texture_id;

SDL_Window* g_display_window;
bool g_game_is_running = true;
bool g_game_is_started = false;
bool g_game_is_ended = false;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
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
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Lunar Lander!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);


    // ––––– green_platforms ––––– //
    GLuint green_platform_texture_id = load_texture(GREEN_PLATFORM_FILEPATH);

    g_state.green_platforms = new Entity[GREEN_PLATFORM_COUNT];

    g_state.green_platforms[GREEN_PLATFORM_COUNT - 1].m_texture_id = green_platform_texture_id;
    g_state.green_platforms[GREEN_PLATFORM_COUNT - 1].set_position(glm::vec3(-3.0f, -2.35f, 0.0f));
    g_state.green_platforms[GREEN_PLATFORM_COUNT - 1].set_width(0.4f);
    g_state.green_platforms[GREEN_PLATFORM_COUNT - 1].update(0.0f, NULL, 0, NULL,0);

    for (int i = 0; i < GREEN_PLATFORM_COUNT - 2; i++)
    {
        g_state.green_platforms[i].m_texture_id = green_platform_texture_id;
        g_state.green_platforms[i].set_position(glm::vec3(i - 1.0f, -3.0f, 0.0f));
        g_state.green_platforms[i].set_width(0.4f);
        g_state.green_platforms[i].update(0.0f, NULL, 0,NULL,0);
    }

    g_state.green_platforms[GREEN_PLATFORM_COUNT - 2].m_texture_id = green_platform_texture_id;
    g_state.green_platforms[GREEN_PLATFORM_COUNT - 2].set_position(glm::vec3(1.5f, -2.0f, 0.0f));
    g_state.green_platforms[GREEN_PLATFORM_COUNT - 2].set_width(0.4f);
    g_state.green_platforms[GREEN_PLATFORM_COUNT - 2].update(0.0f, NULL, 0,NULL,0);

    //-------red_platforms-----//

    GLuint red_platform_texture_id = load_texture(RED_PLATFORM_FILEPATH);

    g_state.red_platforms = new Entity[RED_PLATFORM_COUNT];

    g_state.red_platforms[RED_PLATFORM_COUNT - 1].m_texture_id = red_platform_texture_id;
    g_state.red_platforms[RED_PLATFORM_COUNT - 1].set_position(glm::vec3(-2.0f, -2.0f, 0.0f));
    g_state.red_platforms[RED_PLATFORM_COUNT - 1].set_width(0.4f);
    g_state.red_platforms[RED_PLATFORM_COUNT - 1].update(0.0f, NULL, 0, NULL,0);

    for (int i = 0; i < RED_PLATFORM_COUNT - 2; i++)
    {
        g_state.red_platforms[i].m_texture_id = red_platform_texture_id;
        g_state.red_platforms[i].set_position(glm::vec3(i + 3.5f, -1.0f, 0.0f));
        g_state.red_platforms[i].set_width(0.4f);
        g_state.red_platforms[i].update(0.0f, NULL, 0,NULL,0);
    }

    for (int i = 2; i < RED_PLATFORM_COUNT - 4 ; i++)
    {
        g_state.red_platforms[i].m_texture_id = red_platform_texture_id;
        g_state.red_platforms[i].set_position(glm::vec3(i - 7.0f, -1.0f, 0.0f));
        g_state.red_platforms[i].set_width(0.4f);
        g_state.red_platforms[i].update(0.0f, NULL, 0,NULL,0);
    }

    g_state.red_platforms[RED_PLATFORM_COUNT - 4].m_texture_id = red_platform_texture_id;
    g_state.red_platforms[RED_PLATFORM_COUNT - 4].set_position(glm::vec3(-0.5f, -1.5f, 0.0f));
    g_state.red_platforms[RED_PLATFORM_COUNT - 4].set_width(0.4f);
    g_state.red_platforms[RED_PLATFORM_COUNT - 4].update(0.0f, NULL, 0,NULL,0);

    g_state.red_platforms[RED_PLATFORM_COUNT - 2].m_texture_id = red_platform_texture_id;
    g_state.red_platforms[RED_PLATFORM_COUNT - 2].set_position(glm::vec3(2.5f, -1.5f, 0.0f));
    g_state.red_platforms[RED_PLATFORM_COUNT - 2].set_width(0.4f);
    g_state.red_platforms[RED_PLATFORM_COUNT - 2].update(0.0f, NULL, 0,NULL,0);
   

    // ––––– PLAYER (FOXY) ––––– //
    // Existing
    g_state.player = new Entity();
    g_state.player->set_position(glm::vec3(0.0f, 2.0f, 0.0f));
    g_state.player->set_movement(glm::vec3(0.0f));
    g_state.player->m_speed = 1.0f;
    g_state.player->set_acceleration(glm::vec3(0.0f, -0.905f, 0.0f));
    g_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);
    g_state.player->fuel = 1000000.0f;

    g_state.player ->m_frames_left = 6;
    g_state.player->m_frames_right = 6;
    g_state.player->m_frames_up = 6;
    g_state.player->m_frames_down = 2;
    g_state.player->m_walking[g_state.player->LEFT] = new int[6] {48, 49, 50, 51, 52, 53};
    g_state.player->m_walking[g_state.player->RIGHT] = new int[6] {48, 49, 50, 51, 52, 53};
    g_state.player->m_walking[g_state.player->UP] = new int[6] { 48, 49, 50, 51, 52, 53 };
    g_state.player->m_walking[g_state.player->DOWN] = new int[2] {24, 25};

    g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->RIGHT];  // start  looking right
    g_state.player->m_animation_frames = g_state.player->m_frames_right;
    g_state.player->m_animation_index = 0;
    g_state.player->m_animation_time = 0.0f;
    g_state.player->m_animation_cols = 6;
    g_state.player->m_animation_rows = 12;
    g_state.player->set_height(0.9f);
    g_state.player->set_width(0.9f);


    //-----BACKGOUND------
    GLuint background_texture_id = load_texture(BACKGROUND_FILEPATH);
    g_state.background = new Entity();
    g_state.background->m_texture_id = background_texture_id;
    g_state.background->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_state.background->set_width(10.0f);
    g_state.background->set_height(10.0f);
    g_state.background->is_background = true;
    g_state.background->update(0.0f, NULL, 0,NULL,0);

    //font 
    g_font_texture_id = load_texture(FONT_FILEPATH);
    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



void process_input()
{
    g_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_game_is_running = false;
                break;

            case SDLK_SPACE:
                g_game_is_started = true;
                break;
            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);


  
    if (key_state[SDL_SCANCODE_LEFT])
    {
        if (g_state.player->is_fueld()) {
            g_state.player->set_acceleration_x(-5.0f);
            g_state.player->fuel -= 1.0f;
            g_state.player->m_animation_frames = g_state.player->m_frames_left;
            g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->LEFT];
        }
       
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        if (g_state.player->is_fueld()) {
            g_state.player->set_acceleration_x(5.0f);
            g_state.player->fuel -= 1.0f;
            g_state.player->m_animation_frames = g_state.player->m_frames_right;
            g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->RIGHT];
        }
        
    }

    if (key_state[SDL_SCANCODE_UP])
    {
        if (g_state.player->is_fueld()) {
            g_state.player->set_acceleration_y(0.08f);
            g_state.player->fuel -= 1.0f;
            g_state.player->m_animation_frames = g_state.player->m_frames_up;
            g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->UP];
        }
        
    }   
    else 
    {
        g_state.player->set_acceleration_y(-0.1f);
        g_state.player->m_animation_frames = g_state.player->m_frames_down;
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->DOWN];
        
    }

    if (glm::length(g_state.player->m_movement) > 1.0f)
    {
        g_state.player->m_movement = glm::normalize(g_state.player->m_movement);
    }
}

void update()
{
    if (g_game_is_started) {
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
            g_state.player->update(FIXED_TIMESTEP, g_state.green_platforms, GREEN_PLATFORM_COUNT, g_state.red_platforms, RED_PLATFORM_COUNT);
            delta_time -= FIXED_TIMESTEP;
        }

        g_accumulator = delta_time;
    }

}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    g_state.background->render(&g_program);

    if (!g_game_is_started) {
        draw_text(&g_program, g_font_texture_id, "PRESS SPACE TO START!", 0.4f, 0.05f,
            glm::vec3(-4.5f, 2.0f, 0.0f));
    }
    if (g_state.player->mission_accomplished && !g_state.player->mission_failed) {
        draw_text(&g_program, g_font_texture_id, "MISSION ACCOMPLISHED:)", 0.4f, 0.01f,
            glm::vec3(-4.25f, 0.0f, 0.0f));
        g_game_is_ended = true;

    }
    if (g_state.player->mission_failed && !g_state.player->mission_accomplished) {
        draw_text(&g_program, g_font_texture_id, "MISSION FAILED (loser)", 0.4f, 0.01f,
            glm::vec3(-4.25f, 0.0f, 0.0f));
        g_game_is_ended = true;

    }
        
    if (!g_game_is_ended) {
        

        for (int i = 0; i < GREEN_PLATFORM_COUNT; i++) g_state.green_platforms[i].render(&g_program);
        for (int i = 0; i < RED_PLATFORM_COUNT; i++) g_state.red_platforms[i].render(&g_program);
        g_state.player->render(&g_program);
    }
    std::string fuel_level = "Fuel:" + std::to_string((int)g_state.player->fuel);
    draw_text(&g_program, g_font_texture_id, fuel_level, 0.3f, 0.0005f,
        glm::vec3(-4.75f, 3.5f, 0.0f));
    


    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] g_state.green_platforms;
    delete[] g_state.red_platforms;
    delete g_state.background;
    delete g_state.player;
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
