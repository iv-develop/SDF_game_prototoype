#include <vector>
#include <string>
#include <unordered_map>
#include "sdf_primitives.h"
#include "Pose.h"

#ifndef MY_SKELETON
#define MY_SKELETON


#define BONETYPE_REGULAR 0
#define BONETYPE_LINE_ATTACHED 1
#define BONETYPE_TRIANGLE_ATTACHED 1
#define BONETYPE_SPRING 2

using namespace std;


struct Bone{
    string name;
    Bone* parent = nullptr;
    vector<Primitive*> meshes;

    mat4 init_transform = EYE4;
    vec4 offset = vec4(0);
    
    BoneTransformBundle transform_bundle;

    int bone_type = BONETYPE_REGULAR;
    
    // for LINE || TRIANGLE
    Bone* a = nullptr;
    vec4 a_offset = vec4(0);
    Bone* b = nullptr;
    vec4 b_offset = vec4(0);
    Bone* c = nullptr;
    vec4 c_offset = vec4(0);
};

class Skeleton{
    public:
    unordered_map<string, Bone> bones;

    void add_bone(){}


    void update_scene(){
        unordered_map<string, BoneTransformBundle> cached_transforms; // todo
        
        for (const auto& pair : bones){
            if (pair.second.bone_type != BONETYPE_REGULAR){ // || pair.second.bone_type != BONETYPE_SPRING
                continue;
            }
            string bone_name = pair.first;
            BoneTransformBundle result_transform;
            Bone* bone = &bones[bone_name];
            Bone* parent = bone->parent;
            vector<Bone*> path;
            path.push_back(bone);
            while (parent != nullptr){ // go to root and make path
                path.push_back(parent);
                parent = parent->parent;
            }
            for (auto it = path.rbegin(); it != path.rend(); ++it) { // collect transforms
                Bone* b = *it;
                result_transform.position += (b->offset + b->transform_bundle.position) * result_transform.transform;// +  * b->transform_bundle.transform * result_transform.position;//result_transform.position;
                result_transform.transform = b->transform_bundle.transform * result_transform.transform;
            }
            for (Primitive* p : bones[bone_name].meshes){ // apply transforms to meshses
                p->position = result_transform.position;
                p->transform = bones[bone_name].init_transform * result_transform.transform; // todo: for mesh init offsets and transforms
            }
        }
        for (const auto& pair : bones){
            Bone bone = pair.second;
            if (bone.bone_type == BONETYPE_REGULAR) continue; 
            // after applying transforms we can get relative position of mesh
            if (bone.bone_type == BONETYPE_LINE_ATTACHED){
                for (Primitive* p : bone.meshes){
                    
                }
            }
            if (bone.bone_type == BONETYPE_TRIANGLE_ATTACHED){
                for (Primitive* p : bone.meshes){
                    
                }
            }

        }
    }

    void apply_pose(Pose pose){
        bones["torso"].transform_bundle = pose.torso;
        bones["torso_"].transform_bundle.transform = EulerZYX(pose.torso_bend, 0, 0);
        bones["head"].transform_bundle = pose.head;
        //bones["head_"].transform_bundle.transform = EulerZYX(0, pose.torso_bend, 0);
        bones["larm"].transform_bundle = pose.left_arm;
        bones["larm_"].transform_bundle.transform = EulerZYX(pose.left_arm_bend, 0, 0);
        bones["rarm"].transform_bundle = pose.right_arm;
        bones["rarm_"].transform_bundle.transform = EulerZYX(pose.right_arm_bend, 0, 0);
        bones["lleg"].transform_bundle = pose.left_leg;
        bones["lleg_"].transform_bundle.transform = EulerZYX(pose.left_leg_bend, 0, 0);
        bones["rleg"].transform_bundle = pose.right_leg;
        bones["rleg_"].transform_bundle.transform = EulerZYX(pose.right_leg_bend, 0, 0);

        bones["litem"].transform_bundle = pose.left_leg;
        bones["ritem"].transform_bundle = pose.right_leg;
        // todo : add spring bones and linebones/tribones (set vert to pos of other bone with offset)
    }
};




#endif
