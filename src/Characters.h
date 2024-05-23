#include <string>
#include "Game.h"
#include "Skeleton.h"

using namespace std;

class Character{
    protected:
    Skeleton skeleton;
    PrimitiveScene mesh_scene;
    public:
    SDF_Frag_Shader shader;
    void apply_pose(Pose pose){
        skeleton.apply_pose(pose);
        skeleton.update_scene();
        shader.set_scene(&mesh_scene);
    }
    Character(){}
    ~Character(){
        shader.destroy();
    }
    virtual void time_step(float delta_time){}
};


class BoxBot : public Character{
    public:
    BoxBot() : Character(){
        Primitive* torso = &mesh_scene.primitives[0];
        *torso = (BoxObject(vec3(0.3))).as_primitive();
        skeleton.bones["torso"] = Bone{
            "torso",
            nullptr,
            {torso},
            with_offset(EYE4, {0, -0.5, 0}),
            vec4(0., 1., 0., 1.)
        };

        Primitive* torso_ = &mesh_scene.primitives[1];
        *torso_ = (BoxObject(vec3(0.4))).as_primitive();
        skeleton.bones["torso_"] = Bone{
            "torso_",
            &skeleton.bones["torso"],
            {torso_},
            with_offset(EYE4, {0, -0.5, 0}),
            vec4(0., 1., 0., 1.)
        };

        Primitive* head = &mesh_scene.primitives[2];
        //*head = (SphereObject( 0.5)).as_primitive();
        *head = (BoxObject(vec3(0.6, 0.5, 0.5))).as_primitive();
        skeleton.bones["head"] = Bone{
            "head",
            &skeleton.bones["torso_"],
            {head},
            with_offset(EYE4, {0, -0.5, 0}),
            vec4(0., 1., 0., 1.)
        };

        Primitive* larm = &mesh_scene.primitives[3];
        *larm = (SphereObject(0.5)).as_primitive();
        skeleton.bones["larm"] = Bone{
            "larm",
            &skeleton.bones["torso_"],
            {larm},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(-1., 1., 0., 1.)
        };

        Primitive* larm_ = &mesh_scene.primitives[4];
        *larm_ = (SphereObject(0.5)).as_primitive();
        skeleton.bones["larm_"] = Bone{
            "larm_",
            &skeleton.bones["larm"],
            {larm_},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0., -1., 0., 1.)
        };

        Primitive* rarm = &mesh_scene.primitives[5];
        *rarm = (SphereObject(0.5)).as_primitive();
        skeleton.bones["rarm"] = Bone{
            "rarm",
            &skeleton.bones["torso_"],
            {rarm},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(1., 1., 0., 1.)
        };

        Primitive* rarm_ = &mesh_scene.primitives[6];
        *rarm_ = (SphereObject(0.5)).as_primitive();
        skeleton.bones["rarm_"] = Bone{
            "rarm_",
            &skeleton.bones["rarm"],
            {rarm_},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0., -1., 0., 1.)
        };

        Primitive* lleg = &mesh_scene.primitives[7];
        *lleg = (SphereObject(0.5)).as_primitive();
        skeleton.bones["lleg"] = Bone{
            "lleg",
            &skeleton.bones["torso"],
            {lleg},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(-0.5, 0., 0., 1.)
        };

        Primitive* lleg_ = &mesh_scene.primitives[8];
        *lleg_ = (SphereObject( 0.5)).as_primitive();
        skeleton.bones["lleg_"] = Bone{
            "lleg_",
            &skeleton.bones["lleg"],
            {lleg_},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0, -1, 0., 1.)
        };

        Primitive* rleg = &mesh_scene.primitives[9];
        *rleg = (SphereObject(0.5)).as_primitive();
        skeleton.bones["rleg"] = Bone{
            "rleg",
            &skeleton.bones["torso"],
            {rleg},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0.5, 0., 0., 1.)
        };

        Primitive* rleg_ = &mesh_scene.primitives[10];
        *rleg_ = (SphereObject(0.5)).as_primitive();
        skeleton.bones["rleg_"] = Bone{
            "rleg_",
            &skeleton.bones["rleg"],
            {rleg_},
            with_offset(EYE4, {0, 0.5, 0}),
            vec4(0, -1, 0., 1.)
        };
        


        for (int i = 0; i < skeleton.bones.size(); i++){
            mesh_scene.ordered_operations[i] =
                PrimitiveOperation{
                    OPERATION_UNION,
                    i-1,
                    i,
                    i,
                    0.
                };
        }

        mesh_scene.operations = skeleton.bones.size();
        mesh_scene.size = skeleton.bones.size();
    }
};

class PolyMorph : public Character{
    float local_time = 0;
    float wrapped_time = float(int(local_time * 200) % (12 * 100)) / 100;
    float a = glm::clamp(wrapped_time,      0.0f, 1.0f);
    float b = glm::clamp(wrapped_time - 2,  0.0f, 1.0f);
    float c = glm::clamp(wrapped_time - 4,  0.0f, 1.0f);
    float d = glm::clamp(wrapped_time - 6,  0.0f, 1.0f);
    float e = glm::clamp(wrapped_time - 8,  0.0f, 1.0f);
    float f = glm::clamp(wrapped_time - 10, 0.0f, 1.0f);
    public:
    PolyMorph() : Character(){
        skeleton.bones["torso"] = Bone{
            "torso",
            nullptr,
            {},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };
        Bone* root = &skeleton.bones["torso"];

        Primitive* shere = &mesh_scene.primitives[0];
        *shere = (SphereObject(1)).as_primitive();
        skeleton.bones["sphere"] = Bone{
            "sphere",
            root,
            {shere},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* line = &mesh_scene.primitives[1];
        *line = (LineObject(vec3(-1), vec3(1), 1)).as_primitive();
        skeleton.bones["line"] = Bone{
            "line",
            root,
            {line},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* cyl = &mesh_scene.primitives[2];
        *cyl = (CylinderObject(vec3(-1), vec3(1), 1)).as_primitive();
        skeleton.bones["cyl"] = Bone{
            "cyl",
            root,
            {cyl},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* box = &mesh_scene.primitives[3];
        *box = (BoxObject(vec3(1))).as_primitive();
        skeleton.bones["box"] = Bone{
            "box",
            root,
            {box},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* orb1 = &mesh_scene.primitives[4];
        *orb1 = (SphereObject(0.5)).as_primitive();
        skeleton.bones["orb1"] = Bone{
            "orb1",
            root,
            {orb1},
            with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400, local_time * 40),
            vec4(0., 0., 0., 1.)
        };

        Primitive* orb2 = &mesh_scene.primitives[5];
        *orb2 = (SphereObject(0.5)).as_primitive();
        skeleton.bones["orb2"] = Bone{
            "orb2",
            root,
            {orb2},
            with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 120, local_time * 40),
            vec4(0., 0., 0., 1.)
        };
        Primitive* orb3 = &mesh_scene.primitives[6];
        *orb3 = (SphereObject(0.5)).as_primitive();
        skeleton.bones["orb3"] = Bone{
            "orb3",
            root,
            {orb3},
            with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 240, local_time * 40),
            vec4(0., 0., 0., 1.)
        };

        Primitive* shere1 = &mesh_scene.primitives[7];
        *shere1 = (SphereObject(1)).as_primitive();
        skeleton.bones["shere1"] = Bone{
            "shere1",
            root,
            {shere1},
            with_offset(EYE4, {0, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        Primitive* shere2 = &mesh_scene.primitives[8];
        *shere2 = (SphereObject(0.5)).as_primitive();
        skeleton.bones["shere2"] = Bone{
            "shere2",
            root,
            {shere2},
            with_offset(EYE4, {0.5, 0, 0}),
            vec4(0., 0., 0., 1.)
        };

        mesh_scene.operations = 9;
        mesh_scene.ordered_operations[0] = PrimitiveOperation{3, 0, 1, 1, a};
        mesh_scene.ordered_operations[1] = PrimitiveOperation{3, 1, 2, 2, b};
        mesh_scene.ordered_operations[2] = PrimitiveOperation{3, 2, 3, 3, c};
        mesh_scene.ordered_operations[3] = PrimitiveOperation{0, 4, 5, 4, 0};
        mesh_scene.ordered_operations[4] = PrimitiveOperation{0, 4, 6, 4, 0};
        mesh_scene.ordered_operations[5] = PrimitiveOperation{3, 3, 4, 6, d};
        mesh_scene.ordered_operations[6] = PrimitiveOperation{1, 8, 7, 7, 0.5};
        mesh_scene.ordered_operations[7] = PrimitiveOperation{3, 6, 7, 8, 0};
        mesh_scene.ordered_operations[8] = PrimitiveOperation{3, 8, 0, 8, f};

        // six stages:
        // SPHERE -> LINE -> CYL -> BOX -> ORBS -> DEATH_STAR
        mesh_scene.size = skeleton.bones.size();
    }
    void time_step(float delta_time){
        local_time += delta_time * 30;
        wrapped_time = float(int(local_time * 200) % (12 * 100)) / 100;
        a = glm::clamp(wrapped_time,      0.0f, 1.0f);
        b = glm::clamp(wrapped_time - 2,  0.0f, 1.0f);
        c = glm::clamp(wrapped_time - 4,  0.0f, 1.0f);
        d = glm::clamp(wrapped_time - 6,  0.0f, 1.0f);
        e = glm::clamp(wrapped_time - 8,  0.0f, 1.0f);
        f = glm::clamp(wrapped_time - 10, 0.0f, 1.0f);
        mesh_scene.ordered_operations[0] = PrimitiveOperation{3, 0, 1, 1, a};
        mesh_scene.ordered_operations[1] = PrimitiveOperation{3, 1, 2, 2, b};
        mesh_scene.ordered_operations[2] = PrimitiveOperation{3, 2, 3, 3, c};
        mesh_scene.ordered_operations[5] = PrimitiveOperation{3, 3, 4, 6, d};
        mesh_scene.ordered_operations[8] = PrimitiveOperation{3, 8, 0, 8, f};
        skeleton.bones["orb1"].init_transform = with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 0, local_time * 40);
        skeleton.bones["orb2"].init_transform = with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 120, local_time * 40);
        skeleton.bones["orb3"].init_transform = with_offset(EYE4, vec3(sin(local_time*15)*0.25 + 0.75 , 0, 0)) * EulerXYZ(0, local_time * 400 + 240, local_time * 40);
    }

};

class BloodKnight : public Character{
    public:
    BloodKnight() : Character(){
        // torso
        Primitive* torso = &mesh_scene.primitives[1];
        mesh_scene.primitives[1].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.01, 0, 0, 0, 0, 0.01, 0, 0, -10, -10, 1);
        mesh_scene.primitives[1].primitive_type = 0;
        mesh_scene.primitives[1].rounding = 0.4;
        mesh_scene.primitives[1].transform = EYE4;

        skeleton.bones["torso"] = Bone{
            "torso",
            nullptr,
            {torso},
            with_offset(EYE4, {0, -0.2, 0}),
            vec4(0., 0., 0., 1.)
        };

        // torso_
        Primitive* torso_ = &mesh_scene.primitives[0];
        mesh_scene.primitives[0].texture_transform = mat4( 0.01, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1);
        mesh_scene.primitives[0].primitive_type = 2;
        mesh_scene.primitives[0].a = vec4(0.75, 0.5, 0.75, 0) * 0.5f;
        mesh_scene.primitives[0].rounding = 0;

        skeleton.bones["torso_"] = Bone{
            "torso_",
            &skeleton.bones["torso"],
            {torso_},
            mat4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -0.5, 0., 1} * EulerZYX(25, 45, 0), //  vec4(0, 0.25, 0.2, 0)
            vec4(0., 0.5, 0., 1.)
        };
        

        // head
        Primitive* head1 = &mesh_scene.primitives[2];
        mesh_scene.primitives[2].texture_transform = mat4(0.51, 0, 0, 0,0, 0.51, 0, 0, 0, 0, 0.17, 0, 0, 0, 0.03, 1);
        mesh_scene.primitives[2].primitive_type = 0;
        mesh_scene.primitives[2].rounding = 0.5;
        skeleton.bones["head"] = Bone{
            "head",
            &skeleton.bones["torso_"],
            {head1},
            with_offset(EYE4, vec4(0, -0.25, 0, 0)),
            vec4(0., 1, 0.45, 1.)
        };

        Primitive* head2 = &mesh_scene.primitives[3];
        mesh_scene.primitives[3].texture_transform = mat4( 1, 0, 0, 0, 0, 0.2, 0, 0, 0, 0, 1, 0, 0, 0.3, 10, 1)  * EulerXYZ(-150, 0, -90);
        mesh_scene.primitives[3].primitive_type = 3;
        mesh_scene.primitives[3].a = vec4(0,  -0.005, 0, 0);
        mesh_scene.primitives[3].b = vec4(0, 0.005, 0, 0);
        mesh_scene.primitives[3].rounding = 0.6;

        skeleton.bones["head2"] = Bone{
            "head2",
            &skeleton.bones["head"],
            {head2},
            with_offset(EulerZYX(90, 0, 90), vec4(0, 0, 0, 0)),
            vec4(0., 0.5, -0.35, 1.)
        };
        
        // rleg
        Primitive* rleg = &mesh_scene.primitives[4];
        mesh_scene.primitives[4].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.1, 0, 0, 0, 0, 0.01, 0, 0, -0.3, -10, 1);
        //mesh_scene.primitives[4].position = vec4(0, 0.75, 0.3, 0) + vec4(0.5, -2.4, -0.5, 0) * 0.5f;
        mesh_scene.primitives[4].primitive_type = 1;
        mesh_scene.primitives[4].a = vec4(0., 0., 0., 0) * 0.5f;
        mesh_scene.primitives[4].b = vec4(0., -1., 0., 0) * 0.5f;
        mesh_scene.primitives[4].rounding = 0.4 * 0.5f;

        skeleton.bones["rleg"] = Bone{
            "rleg",
            &skeleton.bones["torso"],
            {rleg},
            with_offset(EYE4, vec4(0, 0, 0, 0)),
            vec4(0.25, 0.0, 0, 1.)
        };

        // rleg_
        Primitive* rleg_ = &mesh_scene.primitives[5];
        mesh_scene.primitives[5].texture_transform = mat4( 0.01, 0, 0, 0, 0, -0.1, 0, 0, 0, 0, 0.01, 0, 0, 0.2, 10, 1);
        mesh_scene.primitives[5].primitive_type = 1;
        mesh_scene.primitives[5].a = vec4(0., 0., 0.1, 0) * 0.5f;
        mesh_scene.primitives[5].b = vec4(0., -0.75, -0., 0) * 0.5f;
        mesh_scene.primitives[5].rounding = 0.6 * 0.5f;

        skeleton.bones["rleg_"] = Bone{
            "rleg_",
            &skeleton.bones["rleg"],
            {rleg_},
            with_offset(EYE4, vec4(0, 0, 0, 1)),
            vec4(0, -0.75, 0, 1.)
        };

        // lleg
        Primitive* lleg = &mesh_scene.primitives[6];
        mesh_scene.primitives[6].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.1, 0, 0, 0, 0, 0.01, 0, 0, -0.3, -10, 1);
        mesh_scene.primitives[6].primitive_type = 1;
        mesh_scene.primitives[6].a = vec4(0., 0., 0., 0);
        mesh_scene.primitives[6].b = vec4(0., -1., 0., 0) * 0.5f;
        mesh_scene.primitives[6].rounding = 0.2;

        skeleton.bones["lleg"] = Bone{
            "lleg",
            &skeleton.bones["torso"],
            {lleg},
            with_offset(EYE4, vec4(0, 0, 0, 1)),
            vec4(-0.25, 0.0, 0, 1.)
        };

        // lleg_
        Primitive* lleg_ = &mesh_scene.primitives[7];
        mesh_scene.primitives[7].texture_transform = mat4( 0.01, 0, 0, 0, 0, -0.1, 0, 0, 0, 0, 0.01, 0, 0, 0.2, 10, 1);
        mesh_scene.primitives[7].primitive_type = 1;
        mesh_scene.primitives[7].a = vec4(0., 0., 0., 0);
        mesh_scene.primitives[7].b = vec4(0., -0.75, 0., 0) * 0.5f;
        mesh_scene.primitives[7].rounding = 0.3;

        skeleton.bones["lleg_"] = Bone{
            "lleg_",
            &skeleton.bones["lleg"],
            {lleg_},
            with_offset(EYE4, vec4(0, 0, 0, 1)),
            vec4(0, -0.75, 0, 1.)
        };

        // rarm
        Primitive* rarm1 = &mesh_scene.primitives[8];
        mesh_scene.primitives[8].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.2, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1)  * EulerXYZ(0, 0, 135);
        //mesh_scene.primitives[8].position = vec4(0, 0.75, 0.3, 0) + vec4(0.9, 0, -0.25, 0) * 0.5f;
        mesh_scene.primitives[8].primitive_type = 0;
        mesh_scene.primitives[8].rounding = 0.7 * 0.5f;
        mesh_scene.primitives[8].transform = EYE4;

        Primitive* rarm2 = &mesh_scene.primitives[9];
        mesh_scene.primitives[9].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.01, 0, 0, 0, 0, 0.01, 0, 0, -0.2, -10, 1);
        mesh_scene.primitives[9].position = vec4(0, 0.75, 0.3, 0) + vec4(1.2, 0, -0.25, 0) * 0.5f;
        mesh_scene.primitives[9].primitive_type = 1;
        mesh_scene.primitives[9].a = vec4(0., 0., 0., 0);
        mesh_scene.primitives[9].b = vec4(0., -1.5, 0.0, 0) * 0.5f;
        mesh_scene.primitives[9].rounding = 0.35 * 0.5f;
        mesh_scene.primitives[9].transform = EYE4;

        skeleton.bones["rarm"] = Bone{
            "rarm",
            &skeleton.bones["torso_"],
            {rarm1},
            with_offset(EYE4, vec4(0, 0, 0, 1)),
            vec4(0.5, 0.5, 0.3, 1.)
        };

        skeleton.bones["rarm1"] = Bone{
            "rarm1",
            &skeleton.bones["rarm"],
            {rarm2},
            with_offset(EYE4, vec4(0, 0, 0, 1)),
            vec4(0, 0, 0, 1.)
        };

        // rarm_
        Primitive* rarm_ = &mesh_scene.primitives[10];
        mesh_scene.primitives[10].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.3, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1);
        mesh_scene.primitives[10].position = vec4(0, 0.75, 0.3, 0) + vec4(1.2, -1.5, -0.25, 0) * 0.5f;
        mesh_scene.primitives[10].primitive_type = 1;
        mesh_scene.primitives[10].a = vec4(0., 0., 0., 0);
        mesh_scene.primitives[10].b = vec4(0., -1, 0.0, 0) * 0.5f;
        mesh_scene.primitives[10].rounding = 0.45 * 0.5f;
        mesh_scene.primitives[10].transform = EYE4;

        skeleton.bones["rarm_"] = Bone{
            "rarm_",
            &skeleton.bones["rarm"],
            {rarm_},
            with_offset(EYE4, vec4(0, 0, 0, 0)),
            vec4(0, -0.75, 0, 1.)
        };

        // larm
        Primitive* larm1 = &mesh_scene.primitives[11];
        mesh_scene.primitives[11].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.2, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1)  * EulerXYZ(0, 0, 135);
        mesh_scene.primitives[11].position = vec4(0, 0.75, 0.3, 0) + vec4(-0.9, 0, -0.25, 0) * 0.5f;
        mesh_scene.primitives[11].primitive_type = 0;
        mesh_scene.primitives[11].rounding = 0.7 * 0.5f;
        mesh_scene.primitives[11].transform = EYE4;

        Primitive* larm2 = &mesh_scene.primitives[12];
        mesh_scene.primitives[12].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.01, 0, 0, 0, 0, 0.01, 0, 0, -0.2, -10, 1);
        mesh_scene.primitives[12].position = vec4(0, 0.75, 0.3, 0) + vec4(-1.2, 0, -0.25, 0) * 0.5f;
        mesh_scene.primitives[12].primitive_type = 1;
        mesh_scene.primitives[12].a = vec4(0., 0., 0., 0);
        mesh_scene.primitives[12].b = vec4(0., -1.5, 0.0, 0) * 0.5f;
        mesh_scene.primitives[12].rounding = 0.35 * 0.5f;
        mesh_scene.primitives[12].transform = EYE4;

        skeleton.bones["larm"] = Bone{
            "larm",
            &skeleton.bones["torso_"],
            {larm1, larm2},
            with_offset(EYE4, vec4(0, 0, 0, 0)),
            vec4(-0.5, 0.5, 0.3, 1.)
        };

        // larm_
        Primitive* larm_ = &mesh_scene.primitives[13];
        mesh_scene.primitives[13].texture_transform = mat4( 0.01, 0, 0, 0, 0, 0.3, 0, 0, 0, 0, 0.01, 0, 0, -0.2, 10, 1);
        mesh_scene.primitives[13].position = vec4(0, 0.75, 0.3, 0) + vec4(-1.2, -1.5, -0.25, 0) * 0.5f;
        mesh_scene.primitives[13].primitive_type = 1;
        mesh_scene.primitives[13].a = vec4(0., 0., 0., 0);
        mesh_scene.primitives[13].b = vec4(0., -1, 0.0, 0) * 0.5f;
        mesh_scene.primitives[13].rounding = 0.45 * 0.5f;
        mesh_scene.primitives[13].transform = EYE4;

        skeleton.bones["larm_"] = Bone{
            "larm_",
            &skeleton.bones["larm"],
            {larm_},
            with_offset(EYE4, vec4(0, 0, 0, 0)),
            vec4(0, -0.75, 0, 1.)
        };
        



        mesh_scene.size = 14;

        mesh_scene.operations = 14;



        mesh_scene.ordered_operations[0] = PrimitiveOperation{1, 2, 3, 3, 0};
        mesh_scene.ordered_operations[1] = PrimitiveOperation{0, 0, 1, 1, 1};
        mesh_scene.ordered_operations[2] = PrimitiveOperation{0, 3, 2, 2, 0.25};
        mesh_scene.ordered_operations[3] = PrimitiveOperation{0, 1, 2, 3, 0};

        mesh_scene.ordered_operations[4] = PrimitiveOperation{0, 3, 4, 4, 0};
        mesh_scene.ordered_operations[5] = PrimitiveOperation{0, 4, 5, 5, 0};
        mesh_scene.ordered_operations[6] = PrimitiveOperation{0, 5, 6, 6, 0};
        mesh_scene.ordered_operations[7] = PrimitiveOperation{0, 6, 7, 7, 0};
        mesh_scene.ordered_operations[8] = PrimitiveOperation{0, 7, 8, 8, 0};
        mesh_scene.ordered_operations[9] = PrimitiveOperation{0, 8, 9, 9, 0};
        mesh_scene.ordered_operations[10] = PrimitiveOperation{0, 9, 10, 10, 0};
        mesh_scene.ordered_operations[11] = PrimitiveOperation{0, 10, 11, 11, 0};
        mesh_scene.ordered_operations[12] = PrimitiveOperation{0, 11, 12, 12, 0};
        mesh_scene.ordered_operations[13] = PrimitiveOperation{0, 12, 13, 13, 0};

        
    }
};

class LivingConstruct : public Character{
    public:
    LivingConstruct() : Character(){
        
    }
};

class WoodEn : public Character{
    public:
    WoodEn() : Character(){
        
    }
};

