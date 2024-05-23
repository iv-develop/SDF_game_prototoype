#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include "cmath"
#include "Functions.h"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <iostream>
#include <variant>
#include <vector>
using namespace std;

#include <chrono>
#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <thread>

#define FIXED 1
#define MOVING 2
#define RIGID 4

#define CAPSULE 1
#define BOX 2
#define PYRAMID 4 // 1 vert
#define RAMP 8 // 2 vert

struct alignas(16) PhysicsPrimitive{
    vec4 position = vec4(0.);
    vec4 velocity = vec4(0.);
    vec4 a = vec4(0.);
    vec4 b = vec4(0.);
    vec4 c = vec4(0.);
    float rounding = 0.;
    float bounciness = 0.1;
    float mass = 1.;
    int type = FIXED;
    int shape = CAPSULE;
    bool y_slopes = false;
    float friction = 0.;
};

class PhysicsSolver{

    // shader
    private:
    GLuint program = glCreateProgram();
    GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
    GLuint input_buffer = 0;
    GLuint output_buffer = 0;
    string file_path;
    string current_src;
    string read_shader(string path=""){
        if (path=="") path = file_path;
        ifstream fileStream(path);
        if (!fileStream.is_open()) {
            cerr << "ERROR! Cant open file!" << endl;
            return "";
        }
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();
    }

    void compile(){
        const GLchar* src = current_src.c_str();
        glShaderSource(compute_shader, 1, &src, NULL);
        glCompileShader(compute_shader);
        GLint success;
        glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(compute_shader, 512, NULL, infoLog);
            std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
            return;
        }
        glAttachShader(program, compute_shader);
        glLinkProgram(program);
        glDeleteShader(compute_shader);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
            return;
        }
        glGenBuffers(1, &input_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input_buffer);
        glGenBuffers(1, &output_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, output_buffer);
    }
    void set_1f(const char* name, float value){
        glUniform1f(glGetUniformLocation(program, name), value);
    }
    void set_3f(const char* name, vec3 value){
        glUniform3f(glGetUniformLocation(program, name), value.x, value.y, value.z);
    }
    public:
    void check_file_updates(){
        string new_src = read_shader();
        if (new_src != current_src){
            current_src = new_src;
            compile();
        }
    }
    PhysicsSolver(string compute_shader_path){
        file_path = compute_shader_path;
    }
    void init(){
        current_src = read_shader();
        compile();
    }

    void draw_line(vec2 pa, vec2 pb, vec2 screen_size, vec3 color){
        vec2 half_sreen = screen_size * 0.5f;
        vec2 ld = pa;
        vec2 ru = pb;
        float a = remap(ld.x, -half_sreen.x, half_sreen.x, -1, 1);
        float b = remap(ld.y, -half_sreen.y, half_sreen.y, -1, 1);
        float c = remap(ru.x, -half_sreen.x, half_sreen.x, -1, 1);
        float d = remap(ru.y, -half_sreen.y, half_sreen.y, -1, 1);


        glBegin(GL_LINES);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(a, b);
            glVertex2f(c, d);
            glColor3f(1., 1., 1.);
        glEnd();
    }

    void draw_box(vec2 pos, vec2 size, vec2 screen_size, vec3 color){
        vec2 half_sreen = screen_size * 0.5f;
        vec2 ld = pos - size * 0.5f;
        vec2 ru = pos + size * 0.5f;
        float a = remap(ld.x, -half_sreen.x, half_sreen.x, -1, 1);
        float b = remap(ld.y, -half_sreen.y, half_sreen.y, -1, 1);
        float c = remap(ru.x, -half_sreen.x, half_sreen.x, -1, 1);
        float d = remap(ru.y, -half_sreen.y, half_sreen.y, -1, 1);


        glBegin(GL_LINES);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(a, b);
            glVertex2f(a, d);
            glVertex2f(a, d);
            glVertex2f(c, d);
            glVertex2f(c, d);
            glVertex2f(c, b);
            glVertex2f(c, b);
            glVertex2f(a, b);
            glColor3f(1., 1., 1.);
        glEnd();

    }
    void draw_capsule(vec2 pos, vec2 size, vec2 screen_size, vec3 color){
        vec2 half_sreen = screen_size * 0.5f;
        vec2 ld = pos - size * vec2(1., 0.5);
        vec2 ru = pos + size * vec2(1., 0.5);

        vec2 uv_pos = remap_vec2(pos, -half_sreen, half_sreen, vec2(-1), vec2(1));
        vec2 radius = remap_vec2(vec2(size.x), -half_sreen, half_sreen, vec2(-1), vec2(1));
        float height = remap(size.y, -half_sreen.y, half_sreen.y, -1, 1);

        float lx = remap(ld.x, -half_sreen.x, half_sreen.x, -1, 1);
        float ly = remap(ld.y, -half_sreen.y, half_sreen.y, -1, 1);
        float rx = remap(ru.x, -half_sreen.x, half_sreen.x, -1, 1);
        float ry = remap(ru.y, -half_sreen.y, half_sreen.y, -1, 1);

        vec2 a = vec2(lx, ly);
        vec2 b = vec2(rx, ry);
        
        float sin30 = 0.5;
        float cos30 = 0.866;
        float sin60 = 0.866;
        float cos60 = 0.5;

        glBegin(GL_LINES);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(a.x, a.y); // ld
            glVertex2f(a.x, b.y); // lu

            glVertex2f(a.x, b.y); // lu
            glVertex2f(uv_pos.x - sin60 * radius.x, uv_pos.y + height * 0.5f + cos60 * radius.y);

            glVertex2f(uv_pos.x - sin60 * radius.x, uv_pos.y + height * 0.5f + cos60 * radius.y);
            glVertex2f(uv_pos.x - sin30 * radius.x, uv_pos.y + height * 0.5f + cos30 * radius.y);

            glVertex2f(uv_pos.x - sin30 * radius.x, uv_pos.y + height * 0.5f + cos30 * radius.y);
            glVertex2f(uv_pos.x, uv_pos.y + height * 0.5f + radius.y);

            glVertex2f(uv_pos.x, uv_pos.y + height * 0.5f + radius.y);
            glVertex2f(uv_pos.x + sin30 * radius.x, uv_pos.y + height * 0.5f + cos30 * radius.y);

            glVertex2f(uv_pos.x + sin30 * radius.x, uv_pos.y + height * 0.5f + cos30 * radius.y);
            glVertex2f(uv_pos.x + sin60 * radius.x, uv_pos.y + height * 0.5f + cos60 * radius.y);

            glVertex2f(uv_pos.x + sin60 * radius.x, uv_pos.y + height * 0.5f + cos60 * radius.y);
            glVertex2f(b.x, b.y); // ru

            glVertex2f(b.x, b.y); // ru
            glVertex2f(b.x, a.y); // rd

            glVertex2f(b.x, a.y); // rd
            glVertex2f(uv_pos.x + sin60 * radius.x, uv_pos.y - height * 0.5f - cos60 * radius.y);

            glVertex2f(uv_pos.x + sin60 * radius.x, uv_pos.y - height * 0.5f - cos60 * radius.y);
            glVertex2f(uv_pos.x + sin30 * radius.x, uv_pos.y - height * 0.5f - cos30 * radius.y);

            glVertex2f(uv_pos.x + sin30 * radius.x, uv_pos.y - height * 0.5f - cos30 * radius.y);
            glVertex2f(uv_pos.x, uv_pos.y - height * 0.5f - radius.y);

            glVertex2f(uv_pos.x, uv_pos.y - height * 0.5f - radius.y);
            glVertex2f(uv_pos.x - sin30 * radius.x, uv_pos.y - height * 0.5f - cos30 * radius.y);


            glVertex2f(uv_pos.x - sin30 * radius.x, uv_pos.y - height * 0.5f - cos30 * radius.y);
            glVertex2f(uv_pos.x - sin60 * radius.x, uv_pos.y - height * 0.5f - cos60 * radius.y);

            glVertex2f(uv_pos.x - sin60 * radius.x, uv_pos.y - height * 0.5f - cos60 * radius.y);
            glVertex2f(a.x, a.y); // ld

            glColor3f(1., 1., 1.);
        glEnd();
    }
    void draw_pyramid(vec2 pos, vec2 plane, vec2 vertex, vec2 screen_size, vec3 color){
        vec2 half_sreen = screen_size * 0.5f;
        vec2 plane_left = pos - vec2{plane.x * 0.5f, -plane.y};
        vec2 plane_right = pos + vec2{plane.x * 0.5f, plane.y};
        vec2 vertex_offset = pos + vertex;
        vec2 uv_plane_l = remap_vec2(plane_left, -half_sreen, half_sreen, vec2(-1), vec2(1));
        vec2 uv_plane_r = remap_vec2(plane_right, -half_sreen, half_sreen, vec2(-1), vec2(1));
        vec2 uv_vert = remap_vec2(vertex_offset, -half_sreen, half_sreen, vec2(-1), vec2(1));
        glBegin(GL_LINES);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(uv_plane_l.x, uv_plane_l.y);
            glVertex2f(uv_plane_r.x, uv_plane_r.y);
            glVertex2f(uv_plane_r.x, uv_plane_r.y);
            glVertex2f(uv_vert.x, uv_vert.y);
            glVertex2f(uv_vert.x, uv_vert.y);
            glVertex2f(uv_plane_l.x, uv_plane_l.y);
            glColor3f(1., 1., 1.);
        glEnd();
    }
    vec3 get_color(int type){
        switch (type){
            case FIXED:
                return vec3(1., 0., 0.);
                break;
            case MOVING:
                return vec3(1., 0., 1.);
                break;
            case RIGID:
                return vec3(0., 0., 1.);
                break;
        }
        return vec3(0.);
    }
    void draw_point(vec2 pos, vec2 screen_size, vec3 color){
        vec2 half_screen = screen_size * 0.5f;
        vec2 pos_uv = remap_vec2(pos, -half_screen, half_screen, {-1, -1}, {1, 1});
        glBegin(GL_POINTS);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(pos_uv.x, pos_uv.y);
            glColor3f(1., 1., 1.);
        glEnd();
    }
    void draw_arrow(vec4 from, vec4 to, vec2 screen_size, vec3 color){
        vec2 half_sreen = screen_size * 0.5f;
        float a = remap(from.x, -half_sreen.x, half_sreen.x, -1, 1);
        float b = remap(from.y, -half_sreen.y, half_sreen.y, -1, 1);
        float c = remap(to.x, -half_sreen.x, half_sreen.x, -1, 1);
        float d = remap(to.y, -half_sreen.y, half_sreen.y, -1, 1);


        glBegin(GL_LINES);
            glColor3f(0, 0, 0);
            glVertex2f(a, b);
            glColor3f(color.r, color.g, color.b);
            glVertex2f(c, d);
            glColor3f(1., 1., 1.);
        glEnd();
    }
    void draw(vec2 screen_size){
        for (PhysicsPrimitive* object: objects){
            vec3 color = get_color(object->type);
            vec2 pos = vec2{object->position.x, object->position.y};
            
            switch (object->shape){
                case CAPSULE:
                    draw_capsule(pos, vec2(object->rounding, object->a.x), screen_size, color);
                    draw_point(pos, screen_size, color);
                    break;
                
                case BOX:
                    draw_box(pos, vec2(object->a.x, object->a.y), screen_size, color);
                    draw_point(pos, screen_size, color);
                    break;
                case PYRAMID:
                    draw_pyramid(pos, {object->a.x, object->a.y}, {object->b.x, object->b.y}, screen_size, color);
                    vec4 vertex1 = object->position - vec4{object->a.x * 0.5f, -object->a.y, 0., 0.};
                    vec4 vertex2 = object->position + vec4{object->a.x * 0.5f, object->a.y, 0., 0.};
                    vec4 vertex3 = object->position + object->b;
                    vec4 center = (vertex1 + vertex2 + vertex3) / 3.0f;
                    draw_point(pos, screen_size, color);
                    break;
            }
        }
    }
    // constructors
    PhysicsPrimitive box(
        vec3 size=vec3(10.)
    ){
        return PhysicsPrimitive{
            vec4(0.),
            vec4(0.),
            vec4(size, 0.),
            vec4(0.),
            vec4(0.),
            0.,
            0.1,
            1.,
            FIXED,
            BOX
        };
    }
    PhysicsPrimitive capsule(
        float height = 10.,
        float radius = 10
    ){
        return PhysicsPrimitive{
            vec4(0.),
            vec4(0.),
            vec4(height),
            vec4(0.),
            vec4(0.),
            radius,
            0.1,
            1.,
            FIXED,
            CAPSULE
        };
    }
    PhysicsPrimitive pyramid( // !WARN! CENTER MUST BE INSIDE PRIMITIVE!
        vec3 base, // xz is plane size, y is y offset 
        vec3 vert // vertex offset
    ){
        return PhysicsPrimitive{
            vec4(0.),
            vec4(0.),
            vec4(base, 0.),
            vec4(vert, 0.),
            vec4(0.),
            0.,
            0.1,
            1.,
            FIXED,
            PYRAMID
        };
    }

    void push(PhysicsPrimitive* p){
        objects.push_back(p);
    }


    vec4 get_aabb_max(PhysicsPrimitive p){
        switch (p.shape) {
            case BOX:
                return p.position + p.a * 0.5f;
                break;
            case CAPSULE:
                return p.position + vec4(p.rounding, p.rounding, p.rounding, 0.) + vec4(0., p.a.x * 0.5, 0., 0.);
                break;
            case PYRAMID:
                return p.position + glm::max(p.a, p.b) * vec4(0.5, 1, 0.5, 1);
                break;
        }
        return vec4(0);
    }

    vec4 get_aabb_min(PhysicsPrimitive p){
        switch (p.shape) {
            case BOX:
                return p.position - p.a * 0.5f;
                break;
            case CAPSULE:
                return p.position - vec4(p.rounding, p.rounding, p.rounding, 0.) - vec4(0., p.a.x * 0.5, 0., 0.);
                break;
            case PYRAMID:
                return p.position - glm::max(p.a, p.b) * vec4(0.5, 1, 0.5, 1);
                break;
        }
        return vec4(0);
    }

    bool intersects(PhysicsPrimitive p1, PhysicsPrimitive p2){ // todo: make for 3d
        vec4 aabb_min_1 = get_aabb_min(p1);
        vec4 aabb_max_1 = get_aabb_max(p1);
        vec4 aabb_min_2 = get_aabb_min(p2);
        vec4 aabb_max_2 = get_aabb_max(p2);
        return aabb_min_1.x <= aabb_max_2.x && aabb_min_1.y <= aabb_max_2.y && aabb_min_1.z <= aabb_max_2.z &&
                aabb_max_1.x >= aabb_min_2.x && aabb_max_1.y >= aabb_min_2.y && aabb_max_1.z >= aabb_min_2.z;
    }

    bool is_point_in_AABB(PhysicsPrimitive p1, vec4 p2){ // todo: make for 3d
        vec4 aabb_min_1 = get_aabb_min(p1);
        vec4 aabb_max_1 = get_aabb_max(p1);
        return aabb_min_1.x <= p2.x && aabb_min_1.y <= p2.y && aabb_min_1.z <= p2.z &&
                aabb_max_1.x >= p2.x && aabb_max_1.y >= p2.y && aabb_max_1.z >= p2.z;
    }

    vec4 apply_rounding(vec4 position, vec4 destination, float rounding){
        if (rounding == 0.){return position;};
        if (destination == position){return position;};
        vec4 vector = destination - position;
        return position + normalize(vector) * rounding;
    };
    vec4 apply_inv_rounding(vec4 position, vec4 destination, float rounding){
        if (rounding == 0.){return position;};
        if (destination == position){return position;};
        vec4 vector = destination - position;
        return position - normalize(vector) * rounding;
    };

    float length_squared(vec2 vec){
        return vec.x * vec.x + vec.y * vec.y;
    }
    float length_squared(vec3 vec){
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }
    float length_squared(vec4 vec){
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
    }

    float clamp(float v, float minv, float maxv){
        return glm::min(glm::max(v, minv), maxv);
    }

    

    vec4 closest_point(PhysicsPrimitive p1, vec4 point){ // todo: make for 3d, split to shape-specific
        switch (p1.shape){
            case CAPSULE:{
                float y = point.y;
                if (p1.position.x == point.x) {
                    if (point.y < p1.position.y) return p1.position - vec4(0., p1.a.x*0.5f, 0., 0.);
                    else                         return p1.position + vec4(0., p1.a.x*0.5f, 0., 0.);
                } else {
                    y = glm::min(y, p1.position.y+p1.a.x*0.5f);
                    y = glm::max(y, p1.position.y-p1.a.x*0.5f);
                }
                return vec4(p1.position.x, y, p1.position.z, p1.position.w);
                break;
            }
            case BOX:{
                vec4 aabb_min = get_aabb_min(p1);
                vec4 aabb_max = get_aabb_max(p1);
                float x = point.x;
                float y = point.y;
                float z = point.z;
                if (is_point_in_AABB(p1, point)){ // todo: make for 3d
                    float dx1 = abs(x - aabb_min.x);
                    float dx2 = abs(x - aabb_max.x);
                    float dy1 = abs(y - aabb_min.y);
                    float dy2 = abs(y - aabb_max.y);
                    if (std::min(dx1, dx2) < std::min(dy1, dy2)){
                        if (dx1 > dx2){
                            x = aabb_max.x;
                        } else {
                            x = aabb_min.x;
                        }
                        return vec4(x, y, z, p1.position.w);
                    } else {
                        if (dy1 > dy2){
                            y = aabb_max.y;
                        } else {
                            y = aabb_min.y;
                        }
                        return vec4(x, y, z, p1.position.w);
                    }
                    return vec4(x, y, z, p1.position.w);
                }
                x = std::min(x, aabb_max.x);
                x = std::max(x, aabb_min.x);
                y = std::min(y, aabb_max.y);
                y = std::max(y, aabb_min.y);
                z = std::min(z, aabb_max.z);
                z = std::max(z, aabb_min.z);
                return vec4(x, y, z, p1.position.w);
                break;
            }
            case PYRAMID:
                //PhysicsPrimitive p1, vec4 point
                vec4 vertex1 = p1.position - vec4{p1.a.x * 0.5f, -p1.a.y, 0., 0.};
                vec4 vertex2 = p1.position + vec4{p1.a.x * 0.5f, p1.a.y, 0., 0.};
                vec4 vertex3 = p1.position + p1.b;
            
                vec4 closestPoint1 = closest_point_on_capped_line(vertex1, vertex2, point);
                vec4 closestPoint2 = closest_point_on_capped_line(vertex1, vertex3, point);
                vec4 closestPoint3 = closest_point_on_capped_line(vertex2, vertex3, point);

                vec4 closest1 = length_squared(point - vertex1) < length_squared(point - vertex2) ? vertex1 : vertex2;
                vec4 maxima = length_squared(point - vertex1) < length_squared(point - vertex2) ? vertex2 : vertex1;
                vec4 closest2 = length_squared(point - vertex3) < length_squared(point - maxima) ? vertex3 : maxima;

                vec4 minima = length_squared(point - closestPoint1) < length_squared(point - closestPoint2) ? closestPoint1 : closestPoint2;
                vec4 result = length_squared(point - closestPoint3) < length_squared(point - minima) ? closestPoint3 : minima;

                return result;
                break;
        }
        return vec4(0);
    }

    vec4 inv_closest_capsule_point(PhysicsPrimitive p1, vec4 point){ // todo: make for 3d
        float y = point.y;
        if (p1.position.x == point.x) {
            if (point.y < p1.position.y) return p1.position + vec4(0., p1.a.x*0.5f, 0., 0.);
            else                         return p1.position - vec4(0., p1.a.x*0.5f, 0., 0.);
        } else {
            y = glm::max(y, p1.position.y-p1.a.x*0.5f);
            y = glm::min(y, p1.position.y+p1.a.x*0.5f);
        }
        return vec4(p1.position.x, y, p1.position.z, p1.position.w);
    }

    vec4 closest_point_on_capped_line(vec4 a, vec4 b, vec4 point){
        vec4 AB = a - b;
        vec4 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        t = clamp(t, 0.0, 1.0);
        return b + t * AB;
    }

    float cross_product(vec2 v1, vec2 v2){
        return v1.x * v2.y - v1.y * v2.x;
    }

    vec4 capsule_vs_line_closest_point(PhysicsPrimitive p1, vec4 a, vec4 b, vec4 center){
        vec4 upper = p1.position;
        upper.y += p1.a.x * 0.5;
        vec4 lower = p1.position;
        lower.y -= p1.a.x * 0.5;
        bool same_side = on_same_side(a, b, p1.position, center);
        if (a.x == b.x || (a.y == b.y && (p1.position.x < glm::min(a.x, b.x) || p1.position.x > glm::max(a.x, b.x)))) { // if x line and x center is out of bounds
            float y = clamp(a.y, lower.y, upper.y);
            return p1.position * vec4(1, 0, 1, 1) + vec4(0, y, 0, 0);
        } else {
            float k = (a.y - b.y) / (a.x - b.x);
            float line_y = b.y + k * (p1.position.x - b.x);
            if (same_side) line_y = b.y - k * (p1.position.x - b.x);
            line_y = glm::min(line_y, glm::max(b.y, a.y));
            line_y = glm::max(line_y, glm::min(b.y, a.y));
            // clamp(line_y, p1.position.y - p1.a.x * 0.5, p1.position.y + p1.a.x * 0.5
            line_y = abs(lower.y - line_y) < abs(upper.y - line_y)?same_side?upper.y:lower.y:same_side?lower.y:upper.y;
            return p1.position * vec4(1, 0, 1, 1) + vec4(0, line_y, 0, 0);
        }
    }

    bool on_same_side(vec2 a, vec2 b, vec2 center, vec2 point){
        float A = a.y - b.y;
        float B = b.x - a.x;
        float C = a.x * b.y - b.x * a.y;
        float r1 = A * center.x + B * center.y + C;
        float r2 = A * point.x + B * point.y + C;
        return (r1 * r2) > 0;
    }

    bool on_same_side_and_line(vec2 a, vec2 b, vec2 center, vec2 point){
        vec2 AB = a - b;
        vec2 AC = point - b;
        float t = dot(AC, AB) / dot(AB, AB);
        return !((t < 0) || (t > 1)) && on_same_side(a, b, center, point);
    }

    void step(float delta, vec2 screen_size){
        // copy
        vector<PhysicsPrimitive> dereferenced;
        for (auto obj: objects) {
            if (obj->type == RIGID){
                obj->velocity += gravity * delta;
                obj->position += obj->velocity * delta;
            }
            dereferenced.push_back(*obj);
        }
        
        for (int a=0;a<objects.size();a++){
            // !ONLY CAPSULES CAN BE RIGID?
            PhysicsPrimitive first = PhysicsPrimitive{*objects[a]};
            if (first.type != RIGID) continue; // iter only RIGID BODIES (update only self)
            vec4 vec = {};
            for (int b=0;b<objects.size();b++){
                if (a==b) continue; // dont intersect self.
                PhysicsPrimitive second = PhysicsPrimitive{*objects[b]};
                if (!intersects(first, second)) continue; // check AABBs
                if (second.shape == CAPSULE){
                    vec4 first_center = first.position;
                    vec4 closest_point2 = closest_point(second, first_center);
                    vec4 closest_point1 = closest_point(first, closest_point2);
                    vec4 result1 = apply_rounding(closest_point1, closest_point2, first.rounding);
                    vec4 result2 = apply_rounding(closest_point2, closest_point1, second.rounding);
                    vec = (result1 - result2) * 1.001f;
                    if (dot(vec, first.position - second.position) > 0) continue; // prevent pushing inside
                    if (second.type != RIGID) first.position -= vec;
                    else first.position -= vec * 0.5f;
                    dereferenced[a] = first; // write changes
                }


                if (second.shape == BOX){
                    vec4 first_center = first.position;
                    vec4 closest_point2 = closest_point(second, first_center);
                    vec3 c = {};

                    vec4 closest_point1 = (is_point_in_AABB(second, first_center)) ? inv_closest_capsule_point(first, closest_point2) : closest_point(first, closest_point2);
                    vec4 result1 = vec4();
                    if ((is_point_in_AABB(second, closest_point1) || (!is_point_in_AABB(second, closest_point1) && is_point_in_AABB(second, first_center)))){
                        if (closest_point1 == closest_point2 || first_center == closest_point2){
                            vec4 to_center = normalize(second.position - first.position);
                            vec4 miss = {};
                            if (abs(to_center.x) > abs(to_center.y)) {// todo: incorrect for non-rectangle ??? maybe correct...
                                miss.x = to_center.x;
                                closest_point1 = first_center + miss;
                            } else {
                                miss.y = to_center.y;
                                closest_point1 = first_center + miss + vec4(0, (miss.y > 0)?first.a.x*0.5:-first.a.x*0.5, 0, 0);
                            };
                        }
                        result1 = apply_inv_rounding(closest_point1, closest_point2, first.rounding);
                    } else {
                        result1 = apply_rounding(closest_point1, closest_point2, first.rounding);
                    }
                    

                    vec4 result2 = apply_rounding(closest_point2, closest_point1, second.rounding);

                    vec = (result1 - result2) * 1.001f;

                    if (dot(vec, first.position - second.position) > 0) continue; // prevent pushing inside
                    if (second.type != RIGID) first.position -= vec;
                    else first.position -= vec * 0.5f;
                    dereferenced[a] = first; // write changes
                }

                if (second.shape == PYRAMID){
                    vec4 vertex1 = second.position - vec4{second.a.x * 0.5f, -second.a.y, 0., 0.};
                    vec4 vertex2 = second.position + vec4{second.a.x * 0.5f, second.a.y, 0., 0.};
                    vec4 vertex3 = second.position + second.b;
                    vec4 center = (vertex1 + vertex2 + vertex3) / 3.0f;
                    vec4 closest_point_a = capsule_vs_line_closest_point(first, vertex1, vertex2, center);
                    vec4 closest_point_b = capsule_vs_line_closest_point(first, vertex1, vertex3, center);
                    vec4 closest_point_c = capsule_vs_line_closest_point(first, vertex2, vertex3, center);

                    vec4 result_a = closest_point_on_capped_line(vertex1, vertex2, closest_point_a);
                    vec4 result_b = closest_point_on_capped_line(vertex1, vertex3, closest_point_b);
                    vec4 result_c = closest_point_on_capped_line(vertex2, vertex3, closest_point_c);

                    //if ((is_point_in_AABB(second, closest_point1) || (!is_point_in_AABB(second, closest_point1) && is_point_in_AABB(second, first_center)))){
                    vec4 result;
                    vec4 point;
                    vec4 vertexA;
                    vec4 vertexB;
                    if (length_squared(closest_point_a - result_a) < length_squared(closest_point_b - result_b)){
                        result = result_a;
                        point = closest_point_a;
                        vertexA = vertex1;
                        vertexB = vertex2;
                    } else {
                        result = result_b;
                        point = closest_point_b;
                        vertexA = vertex1;
                        vertexB = vertex3;
                    }
                    if (length_squared(closest_point_c - result_c) < length_squared(point - result)){
                        result = result_c;
                        point = closest_point_c;
                        vertexA = vertex2;
                        vertexB = vertex3;
                    }
                    //vec2 a, vec2 b, vec2 point1, vec2 point2
                    vec4 rounded = vec4();
                    if ((on_same_side_and_line(vertexA, vertexB, center, point) || (!on_same_side_and_line(vertexA, vertexB, center, point) && on_same_side_and_line(vertexA, vertexB, center, first.position)))){
                        rounded = apply_inv_rounding(point, result, first.rounding);
                    } else {
                        if (point == result){
                            draw_capsule(point, {6, 0}, screen_size, {1, 0, 1});
                            vec4 to_center = normalize(center - first.position);
                            vec4 miss = {};
                            if (abs(to_center.x) > abs(to_center.y)) { // todo: incorrect for non-rectangle
                                miss.x = to_center.x;
                                point = first.position + miss;
                            } else {
                                miss.y = to_center.y;
                                point = first.position + miss + vec4(0, (miss.y > 0)?first.a.x*0.5:-first.a.x*0.5, 0, 0);
                            };
                        }
                        rounded = apply_rounding(point, result, first.rounding);
                    }
                    vec = (rounded - result) * 1.001f;
                    if (length_squared(second.position - (first.position - vec)) < length_squared(second.position - first.position)) continue; // prevent pushing inside
                    if (second.type != RIGID){
                        if (first.y_slopes && (abs(vec.y) > 0.1) ) {
                            float x = vec.x;
                            float y = vec.y;
                            float aspect = x / y;
                            float c = sqrt(x*x + y*y);
                            float d = aspect*c;
                            float result = sqrt(c*c+d*d);
                            first.position.y += (vec.y > 0)?-result:result;
                        } else first.position -= vec;
                    } else {
                        first.position -= vec * 0.5f;
                    }
                    dereferenced[a] = first; // write changes
                }
                if (second.type == RIGID){
                    vec4 vel = first.velocity;
                    vec4 norm = normalize(-vec); // almost in all cases it will be right
                    vec2 projected = (dot(vel, norm) / dot(norm, norm)) * vec2(norm);
                    vec4 y_component = vec4(projected, 0, 0);
                    vec4 plane_component = vel - vec4(projected, 0, 0);

                    if (dot(y_component, vel) > 0) y_component = -y_component;

                    float avg_bounciness = (first.bounciness + second.bounciness) * 0.5;

                    float energy_amount = second.mass / (first.mass + second.mass);

                    vel = normalize(y_component + plane_component) * (length(first.velocity) + length(second.velocity)) * avg_bounciness * energy_amount;
                    if (dot(second.position - first.position, first.velocity) < 0) continue;
                    first.velocity = vel;
                    dereferenced[a] = first; // write changes


                    //vec4 new_vel = y_component * first.bounciness + plane_component * first.friction;
                    

                    
                } else {
                    if (vec != vec4(0.)){
                        vec4 vel = first.velocity;
                        vec4 norm = normalize(-vec); // almost in all cases it will be right
                        vec2 projected = (dot(vel, norm) / dot(norm, norm)) * vec2(norm);
                        vec4 y_component = vec4(projected, 0, 0);
                        vec4 plane_component = vel - vec4(projected, 0, 0);

                        //draw_arrow(first.position, first.position + vel * 20.0f, screen_size, {1, 0, 0});
                        //draw_arrow(first.position, first.position + norm * 20.0f, screen_size, {0, 1, 0});
                        //draw_arrow(first.position, first.position + vec4(projected * 20.0f, 0, 0), screen_size, {0, 0, 1});
                        
                        if (dot(y_component, vel) > 0) y_component = -y_component;

                        //first.velocity = y_component * first.bounciness + plane_component * first.friction;
                        first.velocity = y_component * first.bounciness + plane_component * first.friction;
                        dereferenced[a] = first; // write changes
                        //draw_arrow(first.position, first.position + normalize(plane_component) * 20.0f, screen_size, {1, 0, 0});
                        //draw_arrow(first.position, first.position + normalize(projected) * 20.0f, screen_size, {0, 0, 1});
                    }
                }
                
                 
            }
        }


        // sync
        for (int i=0;i<objects.size();i++){
            *objects[i] = dereferenced[i];
        }
        dereferenced.clear();
        /*glUseProgram(program);
        vector<PhysicsPrimitive> dereferenced;
        for (auto obj: objects) dereferenced.push_back(*obj);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, input_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, objects.size() * sizeof(PhysicsPrimitive), dereferenced.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, objects.size() * sizeof(PhysicsPrimitive), nullptr, GL_STATIC_DRAW);
        set_1f("delta", delta);
        set_3f("gravity", gravity);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, output_buffer);
        glDispatchCompute(objects.size(), 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        vector<PhysicsPrimitive> new_objects(objects.size());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, output_buffer);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, objects.size() * sizeof(PhysicsPrimitive), &new_objects[0]);
        for (int i=0;i<objects.size();i++){
            cout << new_objects[i].type << endl;
            *objects[i] = new_objects[i];
        }
        dereferenced.clear();*/
    }

    // physics logic
    private:
    vec4 gravity = vec4(0., -9.8, 0., 0.);
    vector<PhysicsPrimitive*> objects;
    public:

};