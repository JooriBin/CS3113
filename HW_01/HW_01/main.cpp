/**
* Author: Joori  Bin Jardan
* Assignment: Simple 2D Scene
* Date due: 2023-09-20, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH  = 640,
              WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
                LEVEL_OF_DETAIL    = 0, // mipmap reduction image level
                TEXTURE_BORDER     = 0; // this value MUST be zero


constexpr char CRASH_FILEPATH[]   = "crash.png",
               TAWNA_FILEPATH[] = "tawna.png";

//-------------------------------------------GLOBAL VAR ANAAD CONST FOR TRSNSFORMATION------------------------------------------------//
constexpr glm::vec3 INIT_SCALE_CRASH= glm::vec3(2.0f, 2.0f, 0.0f),
                    INIT_SCALE_TAWNA = glm::vec3(1.0f * 0.5f, 1.91f * 0.5f, 0.0f),
                    INIT_POS_CRASH = glm::vec3(2.0f, 0.0f, 0.0f),
                    INIT_POS_TAWNA = glm::vec3(-2.0f, 0.0f, 0.0f);

constexpr float ROT_INCREMENT = 1.0f;
constexpr float speed = 1.0f;

//acummulators
glm::vec3 scale_vector_tawna(INIT_SCALE_TAWNA.x, INIT_SCALE_TAWNA.y, INIT_SCALE_TAWNA.z);
constexpr float CRASH_RADIUS = 2.0f;// radius of the circle
constexpr float ORBIT_SPEED = 1.0f;  // rotational speed
float       g_angle = 0.0f;     // current angle
float       g_crash_x_offset = 0.0f, // current x and y coordinates
            g_crash_y_offset = 0.0f;
//-------------------------------------------GLOBAL VAR ANAAD CONST FOR TRSNSFORMATION------------------------------------------------//





SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
          g_crash_matrix,
          g_tawna_matrix,
          g_projection_matrix;

float g_previous_ticks = 0.0f;

glm::vec3 g_rotation_crash   = glm::vec3(0.0f, 0.0f, 0.0f),
          g_rotation_tawna = glm::vec3(0.0f, 0.0f, 0.0f);

GLuint g_crash_texture_id,
       g_tawna_texture_id;


GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}


void initialise()
{
    // Initialise video and joystick subsystems
    SDL_Init(SDL_INIT_VIDEO);
    
    g_display_window = SDL_CreateWindow("Hello, Textures!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_crash_matrix       = glm::mat4(1.0f);
    g_tawna_matrix     = glm::mat4(1.0f);
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_crash_texture_id   = load_texture(CRASH_FILEPATH);
    g_tawna_texture_id = load_texture(TAWNA_FILEPATH);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
}


void update()
{
    /* Delta time calculations */
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    /* Game logic */
    g_rotation_crash.y += ROT_INCREMENT * delta_time;
    scale_vector_tawna.x += (INIT_SCALE_TAWNA.x * 0.5) * delta_time;
    scale_vector_tawna.y += (INIT_SCALE_TAWNA.y * 0.5) * delta_time;

    
    //——————————— ORBIT TRANSFORMATIONS ——————————— //
    g_angle += ORBIT_SPEED * delta_time; 
    g_crash_x_offset = CRASH_RADIUS * glm::cos(g_angle);
    g_crash_y_offset = CRASH_RADIUS * glm::sin(g_angle);

    /* Model matrix reset */
    g_crash_matrix   = glm::mat4(1.0f);
    g_tawna_matrix = glm::mat4(1.0f);
    
    /* Transformations */
    g_crash_matrix = glm::translate(g_crash_matrix, glm::vec3(g_crash_x_offset, g_crash_y_offset, 0.0f));
    g_tawna_matrix = glm::translate(g_crash_matrix, glm::vec3(g_crash_x_offset, g_crash_y_offset, 0.0f));
    g_crash_matrix = glm::rotate(g_crash_matrix,
                                 g_rotation_crash.y,
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    g_crash_matrix = glm::scale(g_crash_matrix, INIT_SCALE_CRASH);

    g_tawna_matrix = glm::scale(g_tawna_matrix, scale_vector_tawna);
}


void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
                          0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                          false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    // Bind texture
    draw_object(g_crash_matrix, g_crash_texture_id);
    draw_object(g_tawna_matrix, g_tawna_texture_id);
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


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
