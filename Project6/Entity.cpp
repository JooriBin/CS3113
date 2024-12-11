#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

void Entity::ai_activate(Entity *player, float delata_time)
{
    switch (m_ai_type)
    {
        case WALKER:
            ai_walk(delata_time, player);
            break;
            
        case GUARD:
            ai_guard(player);
            break;
        case FINALBOSS:
            ai_guard(player);
            break;
        case JUMPER:
            ai_jump();
            break;
        case SHOOTER:
            ai_shoot(delata_time,player);
            
        default:
            break;
    }
}

void Entity::ai_jump()
{
    if (m_velocity.y == 0) {
        m_velocity.y += m_jumping_power;
    }

}
void Entity::ai_shoot(float delta_time, Entity* player) {
    float x_distance = fabs(m_position.x - player->m_position.x);
    float y_distance = fabs(m_position.y - player->m_position.y);

    float shooting_range_x = 2.0f; // AI will shoot if player is within 1 unit on x-axis
    float shooting_range_y = 6.0f; // AI will shoot if player is within 0.5 units on y-axis

     //Check if the player is within the AI's shooting range
    if (x_distance <= shooting_range_x && y_distance <= shooting_range_y) {
        shoot(); // Trigger shooting
    }
    update_bullets(delta_time, player, 1);
    



}

void Entity::ai_walk(float delta_time, Entity* player)
{

    if (m_position.x > 9.0f && m_movement.x > 0) {
        m_movement = glm::vec3(-0.5f, 0.0f, 0.0f); // Move left
    }
    else if (m_position.x < 1.0f && m_movement.x < 0) {
        m_movement = glm::vec3(0.5f, 0.0f, 0.0f); // Move right
    }
    shoot(); 
    update_bullets(delta_time, player, 1);
}

void Entity::ai_guard(Entity *player)
{
    switch (m_ai_state) {
        case IDLE:
            if (glm::distance(m_position, player->get_position()) < 3.0f) m_ai_state = WALKING;
            break;
            
        case WALKING:
            if (m_position.x > player->get_position().x) {
                if(m_position.y > player->get_position().y)
                    m_movement = glm::vec3(-1.0f, -1.0f, 0.0f);
                else {
                    m_movement = glm::vec3(-1.0f, 1.0f, 0.0f);
                }
            } else {
                if(m_position.y < player->get_position().y)
                m_movement = glm::vec3(1.0f,1.0f, 0.0f);
                else {
                    m_movement = glm::vec3(1.0f, -1.0f, 0.0f);

                }
            }

            break;
            
        case ATTACKING:
            break;
            
        default:
            break;
    }
}

// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(0), m_velocity(0.0f), m_acceleration(0.0f), m_width(0.0f), m_height(0.0f)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < SECONDS_PER_FRAME; ++i)
        for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

// Parameterized constructor
Entity::Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, int walking[4][6], float animation_time,
    int animation_frames, int animation_index, int animation_cols,
    int animation_rows, float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed),m_acceleration(acceleration), m_jumping_power(jump_power), m_animation_cols(animation_cols),
    m_animation_frames(animation_frames), m_animation_index(animation_index),
    m_animation_rows(animation_rows), m_animation_indices(nullptr),
    m_animation_time(animation_time), m_texture_id(texture_id), m_velocity(0.0f),
    m_width(width), m_height(height), m_entity_type(EntityType)
{
    face_right();
    set_walking(walking);
}

// Simpler constructor for partial initialization
Entity::Entity(GLuint texture_id, float speed,  float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height),m_entity_type(EntityType)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < SECONDS_PER_FRAME; ++i)
        for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}
Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AIState AIState): m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height),m_entity_type(EntityType), m_ai_type(AIType), m_ai_state(AIState)
{
// Initialize m_walking with zeros or any default value
for (int i = 0; i < SECONDS_PER_FRAME; ++i)
    for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

Entity::~Entity() { }

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void const Entity::check_collision_y(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0)
            {
                m_position.y   -= y_overlap;
                m_velocity.y    = 0;

                // Collision!
                m_collided_top  = true;
            } else if (m_velocity.y < 0)
            {
                m_position.y      += y_overlap;
                m_velocity.y       = 0;

                // Collision!
                m_collided_bottom  = true;
            }
        }
    }
}

void const Entity::check_collision_x(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
            if (m_velocity.x > 0)
            {
                m_position.x     -= x_overlap;
                m_velocity.x      = 0;

                // Collision!
                m_collided_right  = true;
                
            } else if (m_velocity.x < 0)
            {
                m_position.x    += x_overlap;
                m_velocity.x     = 0;
 
                // Collision!
                m_collided_left  = true;
            }
        }
    }
}

void const Entity::check_collision_y(Map *map)
{
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    
    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    
    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
            m_position.y += penetration_y;
            m_velocity.y = 0;
            m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
        
    }
}

void const Entity::check_collision_x(Map *map)
{
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left  = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}
void Entity::update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count, Map *map)
{
    if (!m_is_active) return;
    if (m_entity_type == ENEMY) ai_activate(player, delta_time);

    if (m_entity_type == KEYS) {
        if (check_collision(player)) {
            deactivate();
            player->m_collected_keys++;
        }

   
    }

    m_collided_top    = false;
    m_collided_bottom = false;
    m_collided_left   = false;
    m_collided_right  = false;
    
    //if (m_entity_type == PLAYER) {
    //    if (m_position.y < -5.0f)
    //        deactivate();
    //}


    
    if (m_animation_indices != NULL)
    {
        if (glm::length(m_movement) != 0)
        {
            m_animation_time += delta_time;
            float frames_per_second = (float) 1 / SECONDS_PER_FRAME;
            
            if (m_animation_time >= frames_per_second)
            {
                m_animation_time = 0.0f;
                m_animation_index++;
                
                if (m_animation_index >= m_animation_frames)
                {
                    m_animation_index = 0;
                }
            }
        }
    }

    
    m_velocity.x = m_movement.x * m_speed;    
    m_velocity.y = m_movement.y * m_speed;
    m_velocity += m_acceleration * delta_time;

    m_position.y += m_velocity.y * delta_time;
    //check_collision_y(collidable_entities, collidable_entity_count);
    check_collision_y(map);
    
    m_position.x += m_velocity.x * delta_time;
    //check_collision_x(collidable_entities, collidable_entity_count);
    check_collision_x(map);


    if (m_entity_type == PLAYER) {
        update_bullets(delta_time, collidable_entities, collidable_entity_count);
        update_bombs(delta_time, collidable_entities, collidable_entity_count);
        check_collision_enemy(collidable_entities, collidable_entity_count);

    }
    
    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    m_model_matrix = glm::scale(m_model_matrix, m_scale);

}


void Entity::render(ShaderProgram* program)
{

    if (m_is_active) {
        render_bullets(program);
        render_bombs(program);
        program->set_model_matrix(m_model_matrix);


        if (m_animation_indices != NULL)
        {
            draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
            return;
        }


        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

        glBindTexture(GL_TEXTURE_2D, m_texture_id);

        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());
        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    }

}


void const Entity::check_collision_enemy(Entity* enemies, int enemy_count)
{
    if (m_entity_type != PLAYER || !m_is_active) return;

    for (int i = 0; i < enemy_count; ++i) {
        Entity& enemy = enemies[i];
        if (!enemy.is_active()) continue;

        // Collision detection
        float x_distance = fabs(m_position.x - enemy.get_position().x) - ((m_width + enemy.m_width) / 2.0f);
        float y_distance = fabs(m_position.y - enemy.get_position().y) - ((m_height + enemy.m_height) / 2.0f);

        if (x_distance < 0 && y_distance < 0) {
            deactivate(); // Deactivate the player
            break;
        }
    }
}

void Entity::shoot() {
    if (m_entity_type == PLAYER) {
        glm::vec3 bullet_velocity(0.0f, 2.0f, 0.0f); 
        m_bullets.emplace_back(m_position, bullet_velocity);
    }
    if (m_entity_type == ENEMY) {
        if (m_shoot_cooldown >= m_shoot_interval) {
            if (m_ai_type == WALKER) {
                glm::vec3 bullet_velocity(0.0f, -0.5f, 0.0f);
                m_bullets.emplace_back(m_position, bullet_velocity);
                m_shoot_cooldown = 0;
            }

            else {
                glm::vec3 bullet_velocity(0.0f, 1.0f, 0.0f);
                m_bullets.emplace_back(m_position, bullet_velocity);
                m_shoot_cooldown = 0;
            }
            
        }
        else {
            m_shoot_cooldown += 0.1f;
        }
        
    }
}



void Entity::update_bullets(float delta_time, Entity* enemies, int enemy_count) {
    for (size_t i = 0; i < m_bullets.size();) {
        Bullet& bullet = m_bullets[i];
        bullet.position += bullet.velocity * delta_time;

        bool bullet_removed = false;

        // Check for collision with each enemy
        for (int j = 0; j < enemy_count; ++j) {
            Entity& enemy = enemies[j];
            if (!enemy.is_active()) continue;

            float x_distance = fabs(bullet.position.x - enemy.get_position().x) - ((bullet.width + enemy.m_width / 2.0f));
            float y_distance = fabs(bullet.position.y - enemy.get_position().y) - ((bullet.height + enemy.m_height / 2.0f));

            if (x_distance < 0 && y_distance < 0) {
                if (enemy.get_ai_type() == FINALBOSS) {
                    if(enemy.boss_health >0)
                        enemy.boss_health -= 5;
                    else {
                        enemy.deactivate();

                    }
                }
                else {
                    enemy.deactivate();
                    enimes_killed++;
                    if (enimes_killed % 10 == 0) {
                        bombs_counter++;
                    }
                }
                    m_bullets.erase(m_bullets.begin() + i);
                    bullet_removed = true;
                    break;
                

            }
        }

        // Remove bullets off-screen
        if (!bullet_removed) {
            if (bullet.position.x <= 0 || bullet.position.x >= 10 || 
                bullet.position.y <= -9 || bullet.position.y > -2.0) {
                m_bullets.erase(m_bullets.begin() + i);
            }
            else {
                ++i;
            }
        }
    }
}


void Entity::render_bullets(ShaderProgram* program) {
    for (const Bullet& bullet : m_bullets) {
        glm::mat4 model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, bullet.position);
        model_matrix = glm::scale(model_matrix, glm::vec3(bullet.width, bullet.height, 1.0f));
        program->set_model_matrix(model_matrix);

        float vertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f };
        float tex_coords[] = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

        glBindTexture(GL_TEXTURE_2D, m_bullet_texture);

        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());
        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    }
}

void Entity::drop_bomb() {
    if (m_entity_type == PLAYER) {
        bombs.emplace_back(m_position, m_bomb_texture); 
    }
}

void Entity::update_bombs(float delta_time, Entity* enemies, int enemy_count) {
    for (size_t i = 0; i < bombs.size();) {
        Bomb& bomb = bombs[i];
        bomb.time_to_explode -= delta_time;

        bomb.animation_time += delta_time;
        if (bomb.animation_time >= bomb.frame_duration) {
            bomb.animation_time = 0.0f;
            bomb.index++;
            if (bomb.index >= bomb.animation_frames) {
                bomb.index = 0; // Loop back to the first frame
            }
        }


        // Explode the bomb
        if (bomb.time_to_explode <= 0.0f && !bomb.exploded) {
            bomb.exploded = true;

            // Check for collisions with enemies
            for (int j = 0; j < enemy_count; ++j) {
                Entity& enemy = enemies[j];
                if (!enemy.is_active()) continue;

                float distance = glm::distance(bomb.position, enemy.get_position());
                if (distance <= bomb.explosion_radius) {
                    enemy.deactivate(); // Deactivate enemies in range
                }
            }
        }

        if (bomb.exploded) {
            bombs.erase(bombs.begin() + i); // Remove exploded bomb
        }
        else {
            ++i;
        }
    }
}

void Entity::render_bombs(ShaderProgram* program) {
    for (Bomb& bomb : bombs) {
        if (!bomb.exploded || bomb.index != 0) {
            glm::mat4 model_matrix = glm::mat4(1.0f);
            model_matrix = glm::translate(model_matrix, bomb.position);
            model_matrix = glm::scale(model_matrix, glm::vec3(bomb.width, bomb.height, 1.0f));
            program->set_model_matrix(model_matrix);

            // Step 1: Calculate the UV location of the indexed frame
            float u_coord = (float)(bomb.index % bomb.animation_cols) / (float)bomb.animation_cols;
            float v_coord = (float)(bomb.index / bomb.animation_cols) / (float)bomb.animation_rows;

            // Step 2: Calculate its UV size
            float width = 1.0f / (float)bomb.animation_cols;
            float height = 1.0f / (float)bomb.animation_rows;

            // Step 3: Just as we have done before, match the texture coordinates to the vertices
            float tex_coords[] =
            {
                u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
                u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
            };

            float vertices[] =
            {
                -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
                -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
            };
            
            // Step 4: And render
            glBindTexture(GL_TEXTURE_2D, bomb.texture_id);

            glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
            glEnableVertexAttribArray(program->get_position_attribute());

            glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
            glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

            glDrawArrays(GL_TRIANGLES, 0, 6);

            glDisableVertexAttribArray(program->get_position_attribute());
            glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
        }
    }
}
bool Entity::check_bomb_droping() {
    if (enimes_killed % 5 == 0 && past_num_of_enimes != enimes_killed) {
        past_num_of_enimes = enimes_killed;
        return true;
    }
    return false;
}





