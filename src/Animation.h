#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <cctype>

#include <json.hpp>

#include "Game.h"
#include "Functions.h"
#include "Pose.h"



namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace std;


#define EASING_INOUTSINE 0
#define EASING_LINEAR 1
#define EASING_OUTQUAD 2
#define EASING_CONSTANT 3
#define EASING_INQUAD 4
#define EASING_INOUTQUAD 5
#define EASING_INSINE 6
#define EASING_OUTSINE 7
#define EASING_OUTQUART 8


class Animation{
    bool is_angle(int property_enum){
        if (property_enum == PROPERTY_PITCH ||
            property_enum == PROPERTY_ROLL ||
            property_enum == PROPERTY_YAW ||
            property_enum == PROPERTY_BEND) return true;
        return false;
    }

    float rad_to_deg(float rad){
        return rad * (180 / M_PI);
    }

    int get_property_enum(string property){
        // switch doesnt work with  string || char*...
        string upper_property = to_upper(property);
        if (upper_property == "X") return PROPERTY_X;
        if (upper_property == "Y") return PROPERTY_Y;
        if (upper_property == "Z") return PROPERTY_Z;
        if (upper_property == "PITCH") return PROPERTY_PITCH;
        if (upper_property == "ROLL") return PROPERTY_ROLL;
        if (upper_property == "YAW") return PROPERTY_YAW;
        if (upper_property == "BEND") return PROPERTY_BEND;
        cerr << "Incorrect property on read stage: " << upper_property << " in " << name << " Linear applied" << endl;
        return PROPERTY_NONE;
    }

    int get_easing_enum(string easing){
        // switch doesnt work with  string || char*...
        string upper_easing = to_upper(easing);
        if (upper_easing.substr(0, 4) == "EASE") upper_easing.erase(0, 4);
        if (upper_easing == "INOUTSINE") return EASING_INOUTSINE;
        if (upper_easing == "LINEAR") return EASING_LINEAR;
        if (upper_easing == "OUTQUAD") return EASING_OUTQUAD;
        if (upper_easing == "CONSTANT") return EASING_CONSTANT;
        if (upper_easing == "INQUAD") return EASING_INQUAD;
        if (upper_easing == "INOUTQUAD") return EASING_INOUTQUAD;
        if (upper_easing == "INOUTSINE") return EASING_INOUTSINE;
        if (upper_easing == "INSINE") return EASING_INSINE;
        if (upper_easing == "OUTSINE") return EASING_OUTSINE;
        if (upper_easing == "OUTQUART") return EASING_OUTQUART;
        cerr << "Incorrect easing on read stage: " << upper_easing << " in " << name << " Linear applied" << endl;
        return EASING_LINEAR;
    }

    int get_bone_enum(string bone){
        // switch doesnt work with  string || char*...
        string lower_bone = to_lower(bone);
        if (lower_bone == "torso") return BONE_TORSO;
        if (lower_bone == "head") return BONE_HEAD;
        if (lower_bone == "leftarm") return BONE_LEFTARM;
        if (lower_bone == "rightarm") return BONE_RIGHTARM;
        if (lower_bone == "leftleg") return BONE_LEFTLEG;
        if (lower_bone == "rightleg") return BONE_RIGHTLEG;
        if (lower_bone == "leftitem") return BONE_LEFTITEM;
        if (lower_bone == "rightitem") return BONE_RIGHTITEM;
        cerr << "Incorrect bone on read stage: " << lower_bone << " in " << name << " None applied" << endl;
        return BONE_NONE;
    }

    float in_out_sine_easing(float start, float end, float v){
        return start + (end - start) * (-(cos(M_PI * v) - 1) / 2);
    }
    float linear_easing(float start, float end, float v){
        return start + (end - start) * v;
    }
    float out_quad_easing(float start, float end, float v){
        return start + (end - start) * (1 - pow(1 - v, 2));
    }
    float constant_easing(float start, float end, float v){
        return start;
    }
    float in_quad_easing(float start, float end, float v){
        return start + (end - start) * pow(v, 2);
    }
    float in_out_quad_easing(float start, float end, float v){
        return start + (end - start) * (v < 0.5 ? 2 * v * v : 1 - pow(-2 * v + 2, 2) / 2);
    }
    float in_sine_easing(float start, float end, float v){
        return start + (end - start) * (1 - cos((v * M_PI) / 2));
    }
    float out_sine_easing(float start, float end, float v){
        return start + (end - start) * sin((v * M_PI) / 2);
    }
    float out_quart_easing(float start, float end, float v){
        return start + (end - start) * (1 - pow(1 - v, 4));
    }
     

/*
#define EASING_EASEINOUTSINE
#define EASING_EASEINSINE
#define EASING_EASEOUTSINE
#define EASING_EASEOUTQUART
*/

    float apply_easing(int type, float start, float end, float v){
        switch (type)
        {
            case EASING_INOUTSINE: return in_out_sine_easing(start, end, v);
            case EASING_LINEAR: return linear_easing(start, end, v);
            case EASING_OUTQUAD: return out_quad_easing(start, end, v);
            case EASING_CONSTANT: return constant_easing(start, end, v);
            case EASING_INQUAD: return in_quad_easing(start, end, v);
            case EASING_INOUTQUAD: return in_out_quad_easing(start, end, v);
            case EASING_INSINE: return in_sine_easing(start, end, v);;
            case EASING_OUTSINE: return out_sine_easing(start, end, v);;
            case EASING_OUTQUART: return out_quart_easing(start, end, v);;
            default:
                cerr << "Incorrect easing on apply stage: " << type << " in " << name << " Linear applied" << endl;
                return linear_easing(start, end, v);
                break;
        }
    }
    public:
    string name;
    bool looped = false;
    bool degrees = false;
    int start_tick;
    int end_tick;
    int stop_tick;
    int return_tick;

    float start_time; // 20 tps
    float current_tick = 0.;
    bool playing = false;

    //          bone_name          bone_property  keys    |tick|easing|value|
    unordered_map<int, unordered_map<int, vector<tuple<int, int, float>>>> moves;
    /*
        {
            bone_name1 -> {
                property1 -> track[key, key]
                property2 -> track[key]
            }
            bone_name2 -> {
                property1 -> track[key, key]
                property2 -> track[key, key, key]
                property3 -> track[key, key]
            }
        }
    */

    string to_upper(string in){
        for (char &c : in) c = toupper(c);
        return in;
    }

    string to_lower(string in){
        for (char &c : in) c = tolower(c);
        return in;
    }

    void write_if_has(json src, bool* val, string field){
        if (src.contains(field)) {
            if (src[field].is_boolean()) {*val = src[field]; return;}
            if (to_lower(src[field]) == "false") {*val = false; return;}
            if (to_lower(src[field]) == "true") {*val = true; return;}
        }
    }
    void write_if_has(json src, string* val, string field){
        if (src.contains(field)) *val = src[field];
    }
    void write_if_has(json src, int* val, string field){
        if (src.contains(field)) *val = src[field];
    }
    void write_if_has(json src, float* val, string field){
        if (src.contains(field)) *val = src[field];
    }

    public:

    Animation(string path){
        // parser
        std::ifstream file(path);
        if (!file.is_open()) {
            cerr << "Cant open path for animation: " << path << endl;
            return;
        }
        string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        try {
            json data = json::parse(file_content);
            auto emote_data = data["emote"];
            write_if_has(data, &name, "name");
            write_if_has(emote_data, &looped, "isLoop");
            write_if_has(emote_data, &start_tick, "beginTick");
            write_if_has(emote_data, &end_tick, "endTick");
            write_if_has(emote_data, &return_tick, "returnTick");
            write_if_has(emote_data, &stop_tick, "stopTick");
            write_if_has(emote_data, &degrees, "degrees");
            for (auto move : emote_data["moves"]){
                int tick = move["tick"];
                int easing = get_easing_enum(move["easing"]);
                for (auto bone : move.items()){
                    // skip all non bone fields
                    // ignore turn cause idk what it is
                    if (bone.key() == "tick" ||
                        bone.key() == "easing" ||
                        bone.key() == "turn" ||
                        bone.key() == "leftItem" ||
                        bone.key() == "rightItem" ||
                        bone.key() == "comment") continue;
                    // unificate
                    int bone_name = get_bone_enum(bone.key());
                    if (bone_name == BONE_NONE) continue;
                    // parse properties values
                    for (auto property : bone.value().items()){
                        if (property.key() == "comment") continue;
                        int property_name = get_property_enum(property.key());
                        if (property_name == PROPERTY_NONE) continue;
                        float property_value = property.value();
                        // convert radians
                        if (!degrees && is_angle(property_name)) property_value = rad_to_deg(property_value);
                        // req auto creates empty vecs and maps. wow.
                        moves[bone_name][property_name].push_back(make_tuple(tick, easing, property_value));
                    }
                }
            }
        } catch (json::parse_error& e) {
            cerr << "Parsing failed: " << e.what() << endl;
            return;
        }
    }

    RawPose get_pose(float tick = -1){
        if (tick == -1){tick = current_tick;}
        RawPose p;
        for (auto pair : moves){
            int bone_name = pair.first;
            auto properties = pair.second;

            float x = NAN; float y = NAN; float z = NAN;
            float roll = NAN; float pitch = NAN; float yaw = NAN;
            float bend = NAN;
            // collect properties
            for (auto property_pair : properties){
                switch (property_pair.first){ // feels so stupid, but idk how to make it better... maybe struct
                    case PROPERTY_X:
                        x = get_value(bone_name, property_pair.first, tick);
                        break;
                    case PROPERTY_Y:
                        y = get_value(bone_name, property_pair.first, tick);
                        break;
                    case PROPERTY_Z:
                        z = get_value(bone_name, property_pair.first, tick);
                        break;
                    case PROPERTY_PITCH:
                        pitch = get_value(bone_name, property_pair.first, tick);
                        break;
                    case PROPERTY_ROLL:
                        roll = get_value(bone_name, property_pair.first, tick);
                        break;
                    case PROPERTY_YAW:
                        yaw = get_value(bone_name, property_pair.first, tick);
                        break;
                    case PROPERTY_BEND:
                        bend = get_value(bone_name, property_pair.first, tick);
                        break;
                }
            }
            float* bone_bend;
            vec3* bone_pos;
            vec3* bone_rot;
            switch (bone_name){
                case BONE_HEAD:
                    bone_bend = &p.head_bend;
                    bone_pos = &p.head_pos;
                    bone_rot = &p.head_rot;
                    break;
                case BONE_TORSO:
                    bone_bend = &p.torso_bend;
                    bone_pos = &p.torso_pos;
                    bone_rot = &p.torso_rot;
                    break;
                case BONE_LEFTARM:
                    bone_bend = &p.left_arm_bend;
                    bone_pos = &p.left_arm_pos;
                    bone_rot = &p.left_arm_rot;
                    break;
                case BONE_RIGHTARM:
                    bone_bend = &p.right_arm_bend;
                    bone_pos = &p.right_arm_pos;
                    bone_rot = &p.right_arm_rot;
                    break;
                case BONE_LEFTLEG:
                    bone_bend = &p.left_leg_bend;
                    bone_pos = &p.left_leg_pos;
                    bone_rot = &p.left_leg_rot;
                    break;
                case BONE_RIGHTLEG:
                    bone_bend = &p.right_leg_bend;
                    bone_pos = &p.right_leg_pos;
                    bone_rot = &p.right_leg_rot;
                    break;
                case BONE_RIGHTITEM:
                    bone_pos = &p.right_item_pos;
                    bone_rot = &p.right_item_rot;
                    break;
                case BONE_LEFTITEM:
                    bone_pos = &p.left_item_pos;
                    bone_rot = &p.left_item_rot;
                    break;
                default:
                    continue;
                    break;
            }
            *bone_bend = bend;
            *bone_pos = {x, y, z};
            *bone_rot = {pitch, yaw, roll};
        }
        return p;
    }

    void start(){
        current_tick = 0.;
        playing = true;
    }

    void pause(){
        playing = false;
    }

    void end(){
        current_tick = 0.;
        playing = false;
    }

    bool is_playing(){
        return playing;
    }

    float get_value(int bone, int property, float tick, bool first_time=false){ // todo: is it zeros at start? yes, but only at first iter
        if (moves.find(bone) != moves.end()){
            auto bone_move = moves[bone];
            if (bone_move.find(property) != bone_move.end()){
                auto bone_properties = bone_move[property];
                if (bone_properties.size() != 0){
                    int start_tick = 0;
                    int end_tick = this->end_tick + 1;
                    int start_easing;
                    float start_value = NAN;
                    float end_value = NAN;
                    if (bone_properties.size() == 0) {
                        tie(start_tick, start_easing, start_value) = bone_properties[0];
                        return start_value;
                    }
                    // at least 2
                    // find nearest ticks and interpolate
                    int temp_tick;
                    int temp_easing;
                    float temp_value;
                    // tick ordering are garanteed
                    for (auto property : bone_properties){
                        tie(temp_tick, temp_easing, temp_value) = property;
                        if (temp_tick >= start_tick && temp_tick <= tick){
                            start_tick = temp_tick;
                            start_easing = temp_easing;
                            start_value = temp_value;
                        }
                        if (temp_tick > tick){ // end found;
                            end_tick = temp_tick;
                            end_value = temp_value;
                            break;
                        }
                    }
                    if (isnan(end_value)) return start_value; // only start found
                    if (isnan(start_value)) { // tick before first encountered value; only end found
                        //return end_value;
                        // interpolate with last
                        tie(temp_tick, temp_easing, temp_value) = bone_properties[bone_properties.size() - 1];
                        start_tick = this->end_tick - temp_tick;
                        start_value = temp_value;
                        start_easing = temp_easing;
                    } 
                    float v = remap(tick, start_tick, end_tick, 0, 1);
                    if (v > 1) v = 1;
                    if (is_angle(property)){ // :D
                        start_value -= 360 * 4;
                        end_value -= 360 * 4;
                    }
                    return apply_easing(start_easing, start_value, end_value, v); // todo: right angle interpolation?
                }
            }
        }
        return 0; // as default
    }

    ~Animation(){
        for (auto m : moves){
            for (auto e : m.second) e.second.clear();
            m.second.clear();
        }
    }

};
