#include "cmath"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
using namespace glm;
using namespace std;

#define EYE4 {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}

#ifndef SDF_INITED
#define SDF_INITED

#define OPERATION_UNION 0
#define OPERATION_SUBTRACTION 1
#define OPERATION_INTERSECTION 2
#define OPERATION_INTERPOLATION 3

struct alignas(16) Primitive{
    mat4x4 transform;
    mat4x4 texture_transform;
    vec4 position;
    vec4 a;
    vec4 b;
    vec4 c;
    int primitive_type; // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
    float rounding;
};

struct alignas(16) PrimitiveOperation{
    int operation_type; // 0 - soft add, 1 - soft subtract, 2 - intersection, 3 - interpolation
    int left_member;
    int right_member;
    int overrides;
    float value;
};

struct PrimitiveScene{
    int size;
    int operations;
    Primitive primitives[32];
    PrimitiveOperation ordered_operations[32];
};

class Object{
    public:
    vec3 position = vec3(0.);
    float rounding = 0.;
    mat4x4 transform = EYE4;
    mat4x4 texture_transform = EYE4;
    void set_translation_offset(vec3 translation_offset){
        transform[3]  = vec4(translation_offset, 1.);
    }
    virtual Primitive as_primitive(){return Primitive{};};
};

class SphereObject : public Object {
    public:
    SphereObject(float radius, vec3 position = vec3(0), mat4x4 transform = EYE4){
        this->position = position;
        this->rounding = radius;
        this->transform = transform;
    };
    Primitive as_primitive(){
        vec4(position, 0.);
        return Primitive{
            transform,
            texture_transform,
            vec4(position, 0.),
            vec4(0.),
            vec4(0.),
            vec4(0.),
            0, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            rounding,
        };
    }
    
};

class BoxObject : public Object {
    vec3 size;
    public:
    BoxObject(vec3 size, vec3 position = vec3(0), mat4x4 transform = EYE4){
        this->transform = transform;
        this->position = position;
        this->size = size;
    };
    Primitive as_primitive(){
        return Primitive{
            transform,
            texture_transform,
            vec4(position, 0.),
            vec4(size, 0.),
            vec4(0.),
            vec4(0.),
            2, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            rounding,
        };
    }
};

class LineObject : public Object{
    vec3 p1;
    vec3 p2;
    public:
    LineObject(vec3 p1, vec3 p2, float radius, vec3 position = vec3(0), mat4x4 transform = EYE4){
        this->transform = transform;
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->rounding = radius;
    };
    Primitive as_primitive(){
        return Primitive{
            transform,
            texture_transform,
            vec4(position, 0.),
            vec4(p1, 0.),
            vec4(p2, 0.),
            vec4(0.),
            1, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            rounding,
        };
    }
};

class CylinderObject : public Object{
    vec3 p1;
    vec3 p2;
    public:
    CylinderObject(vec3 p1, vec3 p2, float radius, vec3 position = vec3(0), mat4x4 transform = EYE4){
        this->transform = transform;
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->rounding = radius;
    };
    Primitive as_primitive(){
        return Primitive{
            transform,
            texture_transform,
            vec4(position, 0.),
            vec4(p1, 0.),
            vec4(p2, 0.),
            vec4(0.),
            3, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            rounding,
        };
    }
};

class TriangleObject : public Object{
    vec3 p1;
    vec3 p2;
    vec3 p3;
    public:
    TriangleObject(vec3 p1, vec3 p2, vec3 p3, float radius, vec3 position = vec3(0), mat4x4 transform = EYE4){
        this->transform = transform;
        this->position = position;
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;
        this->rounding = radius;
    };
    Primitive as_primitive(){
        return Primitive{
            transform,
            texture_transform,
            vec4(position, 0.),
            vec4(p1, 0.),
            vec4(p2, 0.),
            vec4(p3, 0.),
            4, // 0 - sphere, 1 - capsule, 2 - box, 3 - cyl, 4 - triangle
            rounding,
        };
    }
};


class ObjectScene {
    public:
    vector<Object*> objects;
    vector<PrimitiveOperation> ordered_operations;
    void update_primitive_scene(PrimitiveScene* scene){
        scene->size = objects.size();
        for (int i = 0; i < scene->size; i++){
            scene->primitives[i] = objects[i]->as_primitive();
        }
        scene->operations = ordered_operations.size();
        for (int i = 0; i < scene->operations; i++){
            scene->ordered_operations[i] = ordered_operations[i];
        }
    };
};


#endif