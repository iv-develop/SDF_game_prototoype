#include <glm/glm.hpp>
#include <string>
using namespace std;
using namespace glm;
#ifndef MY_FUNCTIONS
#define MY_FUNCTIONS

#define EYE4 {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}

mat4x4 EulerXYZ(float anglex, float angley, float anglez){
    anglex = anglex / 180. * 3.1415;
    angley = angley / 180. * 3.1415;
    anglez = anglez / 180. * 3.1415;
    mat4x4 rotmatx = mat4x4(
           vec4(1., 0., 0., 0.),
           vec4(0., cos(anglex), -sin(anglex), 0.),
           vec4(0., sin(anglex), cos(anglex), 0.),
           vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmaty = mat4x4(
            vec4(cos(angley), 0., sin(angley), 0.),
            vec4(0., 1., 0., 0.),
            vec4(-sin(angley), 0., cos(angley), 0.),
            vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmatz = mat4x4(
            vec4(cos(anglez), -sin(anglez), 0., 0.),
            vec4(sin(anglez), cos(anglez), 0., 0.),
            vec4(0., 0., 1., 0.),
            vec4(0., 0., 0., 1.)
    );
    return rotmatx * rotmaty * rotmatz;
}

mat4x4 EulerZYX(float anglex, float angley, float anglez){
    anglex = anglex / 180. * 3.1415;
    angley = angley / 180. * 3.1415;
    anglez = anglez / 180. * 3.1415;
    mat4x4 rotmatx = mat4x4(
           vec4(1., 0., 0., 0.),
           vec4(0., cos(anglex), -sin(anglex), 0.),
           vec4(0., sin(anglex), cos(anglex), 0.),
           vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmaty = mat4x4(
            vec4(cos(angley), 0., sin(angley), 0.),
            vec4(0., 1., 0., 0.),
            vec4(-sin(angley), 0., cos(angley), 0.),
            vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmatz = mat4x4(
            vec4(cos(anglez), -sin(anglez), 0., 0.),
            vec4(sin(anglez), cos(anglez), 0., 0.),
            vec4(0., 0., 1., 0.),
            vec4(0., 0., 0., 1.)
    );
    return rotmatz * rotmaty * rotmatx;
}

mat4x4 EulerYXZ(float anglex, float angley, float anglez){
    anglex = anglex / 180. * 3.1415;
    angley = angley / 180. * 3.1415;
    anglez = anglez / 180. * 3.1415;
    mat4x4 rotmatx = mat4x4(
           vec4(1., 0., 0., 0.),
           vec4(0., cos(anglex), -sin(anglex), 0.),
           vec4(0., sin(anglex), cos(anglex), 0.),
           vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmaty = mat4x4(
            vec4(cos(angley), 0., sin(angley), 0.),
            vec4(0., 1., 0., 0.),
            vec4(-sin(angley), 0., cos(angley), 0.),
            vec4(0., 0., 0., 1.)
    );
    mat4x4 rotmatz = mat4x4(
            vec4(cos(anglez), -sin(anglez), 0., 0.),
            vec4(sin(anglez), cos(anglez), 0., 0.),
            vec4(0., 0., 1., 0.),
            vec4(0., 0., 0., 1.)
    );
    return rotmaty * rotmatx * rotmatz;
}

double remap(double value, double fromLow, double fromHigh, double toLow, double toHigh) {
    double normalized = (value - fromLow) / (fromHigh - fromLow);
    return toLow + normalized * (toHigh - toLow);
}

vec2 remap_vec2(vec2 value, vec2 fromLow, vec2 fromHigh, vec2 toLow, vec2 toHigh) {
    vec2 normalized = (value - fromLow) / (fromHigh - fromLow);
    return toLow + normalized * (toHigh - toLow);
}

mat4 with_offset(mat4 mat, vec3 offset){
    mat[3] = vec4(offset, mat[3][3]);
    return mat;
}

string to_string(vec4 vec){
    return "{" + to_string(vec.x) + ", " + to_string(vec.y) + ", " + to_string(vec.z) + ", " + to_string(vec.w) + "}";
}

string to_string(vec3 vec){
    return "{" + to_string(vec.x) + ", " + to_string(vec.y) + ", " + to_string(vec.z) + "}";
}

string to_string(vec2 vec){
    return "{" + to_string(vec.x) + ", " + to_string(vec.y) + "}";
}


class Game;
Game* game = nullptr;

#endif