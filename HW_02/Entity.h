#include <vector>
class Entity
{
private:
    // Removed individual animation arrays

    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    
    glm::mat4 m_model_matrix;
    
    float     m_speed;

    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    
public:
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 4;
public:
    //// Colliding
    bool m_collided_top = false;
    bool m_collided_bottom = false;
    //losing
    bool m_collided_left = false;
    bool m_collided_right = false;

    int number_of_balls = 1;
    bool single_player = false;
    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, float speed, glm::vec3 scale); // Simpler constructor
    ~Entity();
    //void update(float delta_time, Entity* collidable_entities);
    void update(float delta_time, Entity** balls, Entity* background, Entity* player2);
    void render(ShaderProgram *program);
    void normalise_movement() { m_movement = glm::normalize(m_movement); }
    void move_up() { m_movement.y = 1.0f;}
    void move_down() { m_movement.y = -1.0f;}
    bool const check_collision(Entity* other) const;
    bool const check_top_collision();
    bool const check_bottom_collision();
    void single_player_mode(Entity* player2);
    bool const end_game(Entity** balls);
    void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text, float font_size, float spacing, glm::vec3 position);



    // ————— GETTERS ————— //
    glm::vec3 const get_position()   const { return m_position;   }
    glm::vec3 const get_movement()   const { return m_movement;   }
    glm::vec3 const get_scale()      const { return m_scale;      }
    GLuint    const get_texture_id() const { return m_texture_id; }
    float     const get_speed()      const { return m_speed;      }
    bool get_collided_top() const { return m_collided_top; }
    bool get_collided_bottom() const { return m_collided_bottom; }


    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position)  { m_position   = new_position;}
    void const set_movement(glm::vec3 new_movement)  { m_movement   = new_movement;     }
    void const set_scale(glm::vec3 new_scale)        { m_scale      = new_scale;        }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id;   }
    void const set_speed(float new_speed)           { m_speed      = new_speed;        }

};
