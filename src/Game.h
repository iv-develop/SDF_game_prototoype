#ifndef MY_GAME
#define MY_GAME

#define STB_IMAGE_IMPLEMENTATION

//#include "header.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>

#include "Functions.h"
#include "sdf_primitives.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <chrono>

#include <unordered_map>

#include <stdio.h>
#include "BdfFont.h"
#include "stb_image.h"



SDL_Window* window;
//SDL_Renderer* renderer;
SDL_GLContext context;

//480 * 270 ? 320 * 240
int TARGET_WIDTH = 320; // always true :party_popper:
int TARGET_HEIGHT = 240; // always true :party_popper:


float TARGET_ASPECT = (float)TARGET_WIDTH / (float)TARGET_HEIGHT;

class Shader{
    protected:
    GLuint program = glCreateProgram();
    string filePath;
    string current_src;
    int width;
    int height;
    string read_shader(string path=""){
        if (path=="") path = filePath;
        ifstream fileStream(path);
        if (!fileStream.is_open()) {
            cerr << "ERROR! Cant open file!" << endl;
            return "";
        }
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();
    }
    virtual void compile(){};
    public:
    ivec2 size(){return ivec2(width, height);}
    
    void set_1f(const char* name, float value){
        glUniform1f(glGetUniformLocation(program, name), value);
    }
    void set_2f(const char* name, float value1, float value2){
        glUniform2f(glGetUniformLocation(program, name), value1, value2);
    }
    void set_4f(const char* name, float value1, float value2, float value3, float value4){
        glUniform4f(glGetUniformLocation(program, name), value1, value2, value3, value4);
    }
};

class PassShader : public Shader {
    public:
    PassShader(){}
};


class Debugger{
    unordered_map<string, tuple<string, string>> lines; // todo: add destroying!
    vector<string> line_order; // todo: add destroying!
    vec2 screen_size = {0, 0};
    void draw_line(int line, string text){
        font_atlas->draw_text(text, {0, screen_size.y - (line + 1) * font_atlas->get_glyph_size().y}, screen_size);
    }
    int startTime = SDL_GetTicks();
    int endTime = 0;
    int frameCount = 0;
    public:
    BDFAtlas* font_atlas = nullptr;
    void init(BDFAtlas* font_atlas){
        this->font_atlas = font_atlas;
    };
    bool enabled = false;
    void register_basic(){
        register_line(string("fps"), string("FPS: "), string("?"));
        register_line(string("ticks"), string("tick: "), string("?"));
        register_line(string("int_scale"), string("int_scale: "), string("?"));
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%ix%i", TARGET_WIDTH, TARGET_HEIGHT);
        register_line(string("resolution"), string("Current resolution: "), string(buffer));
        snprintf(buffer, sizeof(buffer), "%ix%i", TARGET_WIDTH, TARGET_HEIGHT);
        register_line(string("target_resolution"), string("Target resolution: "), string(buffer));
        int x, y;
        SDL_GetWindowSize(window, &x, &y);
        snprintf(buffer, sizeof(buffer), "%ix%i", x, y);
        register_line(string("window_size"), string("Window size: "), string(buffer));
    }
    void update_basic(){
        char buffer[20];
        endTime = SDL_GetTicks();
        frameCount++;
        if (endTime - startTime >= 200) {
            float fps = frameCount / ((endTime - startTime) / 1000.0f);
            snprintf(buffer, sizeof(buffer), "%i", (int)round(fps));
            update_line(string("fps"), string(buffer));

            // Reset timing variables
            startTime = endTime;
            frameCount = 0;
        }
        snprintf(buffer, sizeof(buffer), "%u", endTime);
        update_line(string("ticks"), string(buffer));
    }

    void register_line(string name, string text, string data){
        if (lines.find(name) == lines.end()){ // if doesnt exists
            lines[name] = make_tuple(text, data);
            line_order.push_back(name);
        } else {
            get<1>(lines[name]) = data;
        }
    }
    void update_line_text(string name, string text){
        get<0>(lines[name]) = text;
    }
    void update_line(string name, string data){
        get<1>(lines[name]) = data;
    }
    void draw(vec2 screen_size){
        this->screen_size = screen_size;
        if (!enabled){return;}
        int line = 0;
        for (const auto& line_name : line_order) {
            string text;
            string data;
            tie(text, data) = lines[line_name];
            draw_line(line, (text + data));
            line += 1;
        }
    };

};

class Game{
    public:
    bool running = true;
    bool true_scalling = true;
    ivec2 screen_pixel_size = ivec2(TARGET_WIDTH, TARGET_HEIGHT);
    ivec4 screen_rect = ivec4(0, 0, TARGET_WIDTH, TARGET_HEIGHT);
    vec4 uv_screen_rect = vec4(-1, -1, 1, 1);

    GLuint screen_texture = 0;
    GLuint screen_normalmap = 0;
    GLuint screen_depth = 0;

    GLuint framebuffer = 0;

    GLuint default_shader = 0;
    
    Debugger debugger;
    Game(){};
    void init(){
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("Simple Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
        context = SDL_GL_CreateContext(window);
        //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        glewExperimental = GL_TRUE;
        glewInit();
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_TEXTURE_2D);
        debugger = Debugger();
        //debugger.init(&font_atlas);
        debugger.register_basic();
        
        update_resolution();

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
        SDL_SetWindowResizable(window, SDL_bool::SDL_TRUE);
        SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "1");
        //SDL_GL_SetSwapInterval(0);

        //TTF_Init();
        //font = TTF_OpenFont("assets/fonts/TinyUnicode.ttf", 16);

        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendEquation(GL_BLEND_COLOR);

    }

    /*typedef void (*Function)(void);
    auto check_fn_time(Function fn){
        auto start = chrono::high_resolution_clock::now();
        fn();
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
        return duration.count();
    }*/

    chrono::high_resolution_clock::time_point timer_start;
    void start_timer(){
        timer_start = chrono::high_resolution_clock::now();
    }
    auto timer_end(){
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - timer_start);
        return duration.count();
    }
    void print_timer_end(){
        cout << "Elapsed time: " << timer_end() << " μs" << endl;
    }



    bool is_running(){
        return running;
    }

    float time(){
        return SDL_GetTicks() / 1000.0f;
    }
    Uint32 currentTime = SDL_GetTicks();
    Uint32 lastTime = currentTime;
    float deltaTime = 0.0f;
    float delta(){
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        return deltaTime;
    }
    float wrapped_delta(){
        return std::min(delta(), 1.0f);
    }
    

    void handle_event(SDL_Event e){
        if (e.type == SDL_KEYDOWN){
            if (e.key.keysym.sym == SDLK_F2){
                true_scalling = !true_scalling;
                update_resolution();
            }
            if (e.key.keysym.sym == SDLK_F3){
                debugger.enabled = !debugger.enabled;
            }
        }
        if (e.type == SDL_QUIT) {
            running = false;
        }
        else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                update_resolution();
                int x, y;
                char buffer[20];
                SDL_GetWindowSize(window, &x, &y);
                snprintf(buffer, sizeof(buffer), "%ix%i", x, y);
                debugger.update_line(string("window_size"), string(buffer));
            }
        }
    }

    void set_default_image_settings(){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    void update_resolution(){ // mamma mia!
        debugger.update_line(string("int_scale"), string(true_scalling?"TRUE":"FALSE"));
        int current_window_w;
        int current_window_h;
        SDL_GetWindowSize(window, &current_window_w, &current_window_h);
        vec2 garanteed_rect;
        float aspect = (float)current_window_w / (float)current_window_h;
        if (aspect <= TARGET_ASPECT){
            garanteed_rect.x = (int)floor((float)current_window_w / (float)TARGET_WIDTH) * TARGET_WIDTH;
            if (garanteed_rect.x == 0 || !true_scalling)
                garanteed_rect.x = current_window_w;
            garanteed_rect.y = garanteed_rect.x / TARGET_ASPECT;
        }
        else {
            garanteed_rect.y = (int)floor((float)current_window_h / (float)TARGET_HEIGHT) * TARGET_HEIGHT;
            if (garanteed_rect.y == 0 || !true_scalling)
                garanteed_rect.y = current_window_h;
            garanteed_rect.x = garanteed_rect.y * TARGET_ASPECT;
        }
        float x_per_px = (float)garanteed_rect.x / (float)TARGET_WIDTH; // ~dpi
        float y_per_px = (float)garanteed_rect.y / (float)TARGET_HEIGHT;
        screen_rect.z = ceil(current_window_w / x_per_px) * x_per_px;
        screen_rect.w = ceil(current_window_h / y_per_px) * y_per_px;
        screen_rect.x = floor(((float)current_window_w - (float)screen_rect.z) / 2.0f);
        screen_rect.y = floor(((float)current_window_h - (float)screen_rect.w) / 2.0f);
        screen_pixel_size = {
            ceil(current_window_w / x_per_px),
            ceil(current_window_h / y_per_px)
        };
        if (screen_texture != 0) glDeleteTextures(1, &screen_texture);
        if (screen_normalmap != 0) glDeleteTextures(1, &screen_normalmap);
        if (screen_depth != 0) glDeleteTextures(1, &screen_depth);
        if (framebuffer != 0) glDeleteFramebuffers(1, &framebuffer);

        glGenTextures(1, &screen_texture);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_pixel_size.x, screen_pixel_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        set_default_image_settings();

        glGenTextures(1, &screen_normalmap);
        glBindTexture(GL_TEXTURE_2D, screen_normalmap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_pixel_size.x, screen_pixel_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        set_default_image_settings();

        glGenTextures(1, &screen_depth);
        glBindTexture(GL_TEXTURE_2D, screen_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_pixel_size.x, screen_pixel_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        set_default_image_settings();

        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, screen_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, screen_depth, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, screen_normalmap, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%ix%i", screen_pixel_size.x, screen_pixel_size.y);
        debugger.update_line(string("resolution"), string(buffer));
    }
    ivec2 get_screen_size(){
        return screen_pixel_size;
    }

    void begin_main(){
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, screen_pixel_size.x, screen_pixel_size.y);
    }
    void draw_fullscreen_quad(){
        glBegin(GL_QUADS);
        glVertex3f(-1, 1, 0);
        glVertex3f(1, 1, 0);
        glVertex3f(1, -1, 0);
        glVertex3f(-1, -1, 0);
        glEnd();
    }
    void end_main(){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screen_rect.z, screen_rect.w);
    }
    void draw_main(){
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(uv_screen_rect.x, uv_screen_rect.y);
        glTexCoord2f(1, 0); glVertex2f(uv_screen_rect.z, uv_screen_rect.y);
        glTexCoord2f(1, 1); glVertex2f(uv_screen_rect.z, uv_screen_rect.w);
        glTexCoord2f(0, 1); glVertex2f(uv_screen_rect.x, uv_screen_rect.w);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void destroy(){
        if (screen_texture != 0) glDeleteTextures(1, &screen_texture);
        if (screen_depth != 0) glDeleteTextures(1, &screen_depth);
        if (screen_normalmap != 0) glDeleteTextures(1, &screen_normalmap);
        SDL_GL_DeleteContext(context);
        //TTF_CloseFont(font);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

class SDF_Shader : public Shader{
    Debugger* debugger;
    GLuint scenebuffer;
    GLuint scene_texture;
    GLuint characterTexture;
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    SDL_Texture* sdl_output_texture = nullptr;
    int width;
    int height;
    vec2 map_texture_size;
    vec3 map_size;
    vec3 position;
    public:
    
    SDF_Shader(string path, Debugger* debugger){
        filePath = path;
        this->debugger = debugger;
        this->debugger->register_line("SDFshader","SDF shader status: ","NOT INITED!");
    }
    
    void init(int w, int h, ivec3 binded_texture_size, GLubyte* binded_texture_data){
        this->debugger->register_line("SDFshader","SDF shader status: ","Initing...");
        width = w;
        height = h;
        current_src = read_shader();
        glGenTextures(1, &scene_texture);
        glBindTexture(GL_TEXTURE_2D, scene_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenTextures(1, &characterTexture);
        glBindTexture(GL_TEXTURE_3D, characterTexture);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB8, binded_texture_size.x, binded_texture_size.y, binded_texture_size.z, 0, GL_RGB, GL_UNSIGNED_BYTE, binded_texture_data);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
        compile();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, characterTexture);
        glUniform1i(glGetUniformLocation(program, "character_texture"), 0);
    }
    void compile(){
        this->debugger->register_line("SDFshader","SDF shader status: ","Compiling...");
        const GLchar* shaderSource = current_src.c_str();
        glShaderSource(computeShader, 1, &shaderSource, NULL);
        glCompileShader(computeShader);
        GLint success;
        glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
            std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
            this->debugger->update_line("SDFshader", string("compilation failed! check console"));
            return;
        }
        glAttachShader(program, computeShader);
        glLinkProgram(program);
        glDeleteShader(computeShader);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
            this->debugger->update_line("SDFshader", string("program linking failed! check console"));
            return;
        }
        glBindImageTexture(0, scene_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glGenBuffers(1, &scenebuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scenebuffer);
        this->debugger->register_line("SDFshader","SDF shader status: ","Compiled!");
    }
    void destroy(){
        glDeleteTextures(1, &scene_texture);
        glDeleteTextures(1, &characterTexture);
        glDeleteBuffers(1, &scenebuffer);
        SDL_DestroyTexture(sdl_output_texture);
        glDeleteProgram(program);
    }
    void use(){
        glUseProgram(program);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene_texture);
        glUniform1i(glGetUniformLocation(program, "destTex"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, characterTexture);
        glUniform1i(glGetUniformLocation(program, "character_texture"), 1);
    }
    
    void check_file_updates(){
        string new_src = read_shader();
        if (new_src != current_src){
            current_src = new_src;
            compile();
        }
    }
    void run(){
        glDispatchCompute(ceil((float)width / 16.), ceil((float)height / 16.), 1);
    }

    void draw(vec2 dst_size){
        ivec3 tile_size = ivec3(16, 10, 8);
        vec2 half_map_size = map_texture_size * 0.5f;
        vec3 dest = position;
        dest.y -= map_size.y;
        dest.y += 1;
        dest *= tile_size;
        
        
        dest += ivec3(8, -5, 4); // centering

        vec2 pos2d = ivec2{1 + dest.x + dest.z, (1 + dest.z - dest.y)};

        glUniform4f(glGetUniformLocation(program, "depth_texture_rect"), 
            remap(pos2d.x-width * 0.5, 0, map_texture_size.x, 0, 1),
            remap(pos2d.y-height * 0.5 + 10, 0, map_texture_size.y, 0, 1),   //idk why 10, inv centering y?
            remap(pos2d.x+width * 0.5, 0, map_texture_size.x, 0, 1),
            remap(pos2d.y+height * 0.5 + 10, 0, map_texture_size.y, 0, 1)    //idk why 10, inv centering y?
        );
        pos2d.y = map_texture_size.y - pos2d.y;
        pos2d = -half_map_size + pos2d;
        pos2d += dst_size * 0.5f;
        

        vec4 dst_rect = {pos2d.x+width/2, pos2d.y+height/2, pos2d.x-width/2, pos2d.y-height/2};

        vec4 dst_uv = {
            remap(dst_rect.x, 0, dst_size.x, -1, 1),
            remap(dst_rect.y, 0, dst_size.y, -1, 1),
            remap(dst_rect.z, 0, dst_size.x, -1, 1),
            remap(dst_rect.w, 0, dst_size.y, -1, 1),
        };
        glBindTexture(GL_TEXTURE_2D, scene_texture);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(dst_uv.x, dst_uv.y);
        glTexCoord2f(1, 0); glVertex2f(dst_uv.z, dst_uv.y);
        glTexCoord2f(1, 1); glVertex2f(dst_uv.z, dst_uv.w);
        glTexCoord2f(0, 1); glVertex2f(dst_uv.x, dst_uv.w);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    void update_map(GLuint depth_texture, vec2 map_texture_size, vec3 map_size){
        this->map_size = map_size;
        this->map_texture_size = map_texture_size;

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glUniform1i(glGetUniformLocation(program, "map_depth"), 2);
        glActiveTexture(GL_TEXTURE0);
    }

    void set_position(vec3 pos){
        position = pos;
        float depth_step = 1. / (float)(map_size.z+1);
        float depth = depth_step * position.z;
        glUniform2f(glGetUniformLocation(program, "self_depth_range"), depth, depth+depth_step);
    }

    void wait(){
        glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
    }

    void set_scene(PrimitiveScene* primitive_scene){
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, scenebuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(*primitive_scene), primitive_scene, GL_DYNAMIC_DRAW);
    }
};

class SDF_Frag_Shader : public Shader{
    GLuint scenebuffer;
    GLuint scene_texture;
    GLuint characterTexture;
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    SDL_Texture* sdl_output_texture = nullptr;
    int width;
    int height;
    vec2 map_texture_size;
    vec3 map_size;
    vec3 position;
    public:
    
    SDF_Frag_Shader(string frag_path="./assets/shaders/sdf_scene.frag"){
        filePath = frag_path;
    }
    
    void init(int w, int h, ivec3 binded_texture_size, GLubyte* binded_texture_data){
        width = w;
        height = h;
        current_src = read_shader();
        glGenTextures(1, &scene_texture);
        glBindTexture(GL_TEXTURE_2D, scene_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenTextures(1, &characterTexture);
        glBindTexture(GL_TEXTURE_3D, characterTexture);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, binded_texture_size.x, binded_texture_size.y, binded_texture_size.z, 0, GL_RGBA, GL_UNSIGNED_BYTE, binded_texture_data);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
        compile();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, characterTexture);
        glUniform1i(glGetUniformLocation(program, "character_texture"), 0);
    }
    void compile(){
        const GLchar* shaderSource = current_src.c_str();
        glShaderSource(frag_shader, 1, &shaderSource, NULL);
        glCompileShader(frag_shader);
        GLint success;
        glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(frag_shader, 512, NULL, infoLog);
            std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
            return;
        }
        
        const GLchar* vert_shaderSource = R"(
            #version 430 core
            in vec2 in_tex;
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec2 texCoord;

            out vec2 fragTexCoord;

            void main() {
                fragTexCoord = texCoord;
                gl_Position = vec4(position, 1.);
            }
        )";
        glShaderSource(vert_shader, 1, &vert_shaderSource, NULL);
        glCompileShader(vert_shader);
        glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(vert_shader, 512, NULL, infoLog);
            std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
            return;
        }
        glAttachShader(program, vert_shader);
        glAttachShader(program, frag_shader);
        glLinkProgram(program);
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
            return;
        }
        glBindImageTexture(0, scene_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glGenBuffers(1, &scenebuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scenebuffer);
        cout << "Compiled!" << endl;
    }
    void destroy(){
        glDeleteTextures(1, &scene_texture);
        glDeleteTextures(1, &characterTexture);
        glDeleteBuffers(1, &scenebuffer);
        SDL_DestroyTexture(sdl_output_texture);
        glDeleteProgram(program);
    }
    void use(){
        glUseProgram(program);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene_texture);
        glUniform1i(glGetUniformLocation(program, "destTex"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, characterTexture);
        glUniform1i(glGetUniformLocation(program, "character_texture"), 1);
    }
    
    void check_file_updates(){
        string new_src = read_shader();
        if (new_src != current_src){
            current_src = new_src;
            compile();
        }
    }
    

    void draw(vec2 dst_size){
        vec3 tile_size = vec3(16, 10, 8);
        vec2 half_map_size = map_texture_size * 0.5f;
        vec3 dest = position;
        dest.y -= map_size.y;
        dest.y += 1;
        dest *= tile_size;
        
        
        dest += vec3(8, -5, 4); // centering

        vec2 pos2d = vec2{1 + dest.x + dest.z, (1 + dest.z - dest.y)};
        glUniform4f(glGetUniformLocation(program, "depth_texture_rect"), 
            remap(pos2d.x-width * 0.5, 0, map_texture_size.x, 0, 1),
            remap(pos2d.y-height * 0.5, 0, map_texture_size.y, 0, 1),
            remap(pos2d.x+width * 0.5, 0, map_texture_size.x, 0, 1),
            remap(pos2d.y+height * 0.5, 0, map_texture_size.y, 0, 1)
        );
        pos2d.y = map_texture_size.y - pos2d.y;
        pos2d = -half_map_size + pos2d;
        pos2d += dst_size * 0.5f;
        

        vec4 dst_rect = {pos2d.x+width* 0.5, pos2d.y+height* 0.5, pos2d.x-width* 0.5, pos2d.y-height* 0.5};

        vec4 dst_uv = {
            remap(dst_rect.x, 0, dst_size.x, -1, 1),
            remap(dst_rect.y, 0, dst_size.y, -1, 1),
            remap(dst_rect.z, 0, dst_size.x, -1, 1),
            remap(dst_rect.w, 0, dst_size.y, -1, 1),
        };
        //glBindTexture(GL_TEXTURE_2D, scene_texture);
        glBegin(GL_QUADS);
        glColor3f(1, 1, 1); 
        glVertexAttrib2f(1, 0, 0); glVertex2f(dst_uv.x, dst_uv.y);
        glVertexAttrib2f(1, 1, 0); glVertex2f(dst_uv.z, dst_uv.y);
        glVertexAttrib2f(1, 1, 1); glVertex2f(dst_uv.z, dst_uv.w);
        glVertexAttrib2f(1, 0, 1); glVertex2f(dst_uv.x, dst_uv.w);
        glEnd();
        glUseProgram(0);
        //glBindTexture(GL_TEXTURE_2D, 0);
    }

    void update_map(GLuint depth_texture, vec2 map_texture_size, vec3 map_size){
        this->map_size = map_size;
        this->map_texture_size = map_texture_size;

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glUniform1i(glGetUniformLocation(program, "map_depth"), 2);
        glActiveTexture(GL_TEXTURE0);
    }

    void set_position(vec3 pos){
        position = pos;
        float depth_step = 1. / (float)(map_size.z+2);
        float depth = depth_step * (position.z+1);
        glUniform2f(glGetUniformLocation(program, "self_depth_range"), depth, depth+depth_step);
    }

    void set_scene(PrimitiveScene* primitive_scene){
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, scenebuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(*primitive_scene), primitive_scene, GL_DYNAMIC_DRAW);
    }
};
#endif