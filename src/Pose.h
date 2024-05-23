#include <unordered_map>

#include "Functions.h"

using namespace std;

#ifndef MY_POSE
#define MY_POSE

#define BONE_TORSO 0
#define BONE_HEAD 1
#define BONE_LEFTARM 2
#define BONE_RIGHTARM 3
#define BONE_LEFTLEG 4
#define BONE_RIGHTLEG 5
#define BONE_RIGHTITEM 6
#define BONE_LEFTITEM 7
#define BONE_NONE -1

#define PROPERTY_X 0
#define PROPERTY_Y 1
#define PROPERTY_Z 2
#define PROPERTY_PITCH 3
#define PROPERTY_ROLL 4
#define PROPERTY_YAW 5
#define PROPERTY_BEND 6
#define PROPERTY_NONE -1

struct BoneTransformBundle{
    mat4 transform = EYE4;
    vec4 position = vec4(0, 0, 0, 1);
    BoneTransformBundle with_relative_position(BoneTransformBundle relative_to){
        return {this->transform, this->position - relative_to.position};
    }
};




struct RawPose{ // pose with NANs instead of zeros if some property is undefined for better pose interpolation.
    vec3 torso_pos = vec3(NAN);
    vec3 torso_rot = vec3(NAN);
    float torso_bend = NAN;
    vec3 head_pos = vec3(NAN);
    vec3 head_rot = vec3(NAN);
    float head_bend = NAN;
    vec3 left_arm_pos = vec3(NAN);
    vec3 left_arm_rot = vec3(NAN);
    float left_arm_bend = NAN;
    vec3 right_arm_pos = vec3(NAN);
    vec3 right_arm_rot = vec3(NAN);
    float right_arm_bend = NAN;
    vec3 left_leg_pos = vec3(NAN);
    vec3 left_leg_rot = vec3(NAN);
    float left_leg_bend = NAN;
    vec3 right_leg_pos = vec3(NAN);
    vec3 right_leg_rot = vec3(NAN);
    float right_leg_bend = NAN;

    vec3 right_item_pos = vec3(NAN);
    vec3 right_item_rot = vec3(NAN);
    vec3 left_item_pos = vec3(NAN);
    vec3 left_item_rot = vec3(NAN);
};

struct Pose{ 
    BoneTransformBundle torso;
    float torso_bend = 0;
    BoneTransformBundle head;
    float head_bend = 0;
    BoneTransformBundle left_arm;
    float left_arm_bend = 0;
    BoneTransformBundle right_arm;
    float right_arm_bend = 0;
    BoneTransformBundle left_leg;
    float left_leg_bend = 0;
    BoneTransformBundle right_leg;
    float right_leg_bend = 0;
    BoneTransformBundle left_item;
    BoneTransformBundle right_item;
};

Pose compute_pose(RawPose pose, float mul = 1./16.){ // replace NAN -> 0 and compute rotmats
    Pose out;
    // todo: right coordinate system for pos
    out.head = {
        EulerXYZ((isnan(pose.head_rot.x)?0:pose.head_rot.x), (isnan(pose.head_rot.y)?0:pose.head_rot.y), (isnan(pose.head_rot.z)?0:pose.head_rot.z)),
        vec4((isnan(pose.head_pos.x)?0:pose.head_pos.x), (isnan(pose.head_pos.y)?0:-pose.head_pos.y), (isnan(pose.head_pos.z)?0:pose.head_pos.z), 0) * mul
    };

    out.torso = {
        EulerXYZ((isnan(pose.torso_rot.x)?0:-pose.torso_rot.x), (isnan(pose.torso_rot.y)?0:-pose.torso_rot.y), (isnan(pose.torso_rot.z)?0:pose.torso_rot.z)),
        vec4((isnan(pose.torso_pos.x)?0:pose.torso_pos.x), (isnan(pose.torso_pos.y)?0:pose.torso_pos.y), (isnan(pose.torso_pos.z)?0:pose.torso_pos.z), 0) * mul
    };

    out.left_arm = {
        EulerXYZ((isnan(pose.left_arm_rot.x)?0:pose.left_arm_rot.x), (isnan(pose.left_arm_rot.y)?0:pose.left_arm_rot.y), (isnan(pose.left_arm_rot.z)?0:pose.left_arm_rot.z)),
        vec4((isnan(pose.left_arm_pos.x)?0:pose.left_arm_pos.x), (isnan(pose.left_arm_pos.y)?0:-pose.left_arm_pos.y), (isnan(pose.left_arm_pos.z)?0:pose.left_arm_pos.z), 0) * mul
    };

    out.right_arm = {
        EulerXYZ((isnan(pose.right_arm_rot.x)?0:pose.right_arm_rot.x), (isnan(pose.right_arm_rot.y)?0:pose.right_arm_rot.y), (isnan(pose.right_arm_rot.z)?0:pose.right_arm_rot.z)),
        vec4((isnan(pose.right_arm_pos.x)?0:pose.right_arm_pos.x), (isnan(pose.right_arm_pos.y)?0:-pose.right_arm_pos.y), (isnan(pose.right_arm_pos.z)?0:pose.right_arm_pos.z), 0) * mul
    };

    out.left_leg = {
        EulerXYZ((isnan(pose.left_leg_rot.x)?0:pose.left_leg_rot.x), (isnan(pose.left_leg_rot.y)?0:pose.left_leg_rot.y), (isnan(pose.left_leg_rot.z)?0:pose.left_leg_rot.z)),
        vec4((isnan(pose.left_leg_pos.x)?0:pose.left_leg_pos.x), (isnan(pose.left_leg_pos.y)?0:-pose.left_leg_pos.y), (isnan(pose.left_leg_pos.z)?0:pose.left_leg_pos.z), 0) * mul
    };

    out.right_leg = {
        EulerXYZ((isnan(pose.right_leg_rot.x)?0:pose.right_leg_rot.x), (isnan(pose.right_leg_rot.y)?0:pose.right_leg_rot.y), (isnan(pose.right_leg_rot.z)?0:pose.right_leg_rot.z)),
        vec4((isnan(pose.right_leg_pos.x)?0:pose.right_leg_pos.x), (isnan(pose.right_leg_pos.y)?0:-pose.right_leg_pos.y), (isnan(pose.right_leg_pos.z)?0:pose.right_leg_pos.z), 0) * mul
    };

    out.left_item = {
        EulerXYZ((isnan(pose.left_item_rot.x)?0:pose.left_item_rot.x), (isnan(pose.left_item_rot.y)?0:pose.left_item_rot.y), (isnan(pose.left_item_rot.z)?0:pose.left_item_rot.z)),
        vec4((isnan(pose.left_item_pos.x)?0:pose.left_item_pos.x), (isnan(pose.left_item_pos.y)?0:-pose.left_item_pos.y), (isnan(pose.left_item_pos.z)?0:pose.left_item_pos.z), 0) * mul
    };

    out.right_item = {
        EulerXYZ((isnan(pose.right_item_rot.x)?0:pose.right_item_rot.x), (isnan(pose.right_item_rot.y)?0:pose.right_item_rot.y), (isnan(pose.right_item_rot.z)?0:pose.right_item_rot.z)),
        vec4((isnan(pose.right_item_pos.x)?0:pose.right_item_pos.x), (isnan(pose.right_item_pos.y)?0:-pose.right_item_pos.y), (isnan(pose.right_item_pos.z)?0:pose.right_item_pos.z), 0) * mul
    };
    out.torso_bend = isnan(pose.torso_bend)?0:pose.torso_bend;
    out.head_bend = isnan(pose.head_bend)?0:pose.head_bend;
    out.left_arm_bend = isnan(pose.left_arm_bend)?0:pose.left_arm_bend;
    out.right_arm_bend = isnan(pose.right_arm_bend)?0:pose.right_arm_bend;
    out.left_leg_bend = isnan(pose.left_leg_bend)?0:pose.left_leg_bend;
    out.right_leg_bend = isnan(pose.right_leg_bend)?0:pose.right_leg_bend;
    return out;
}
#endif
