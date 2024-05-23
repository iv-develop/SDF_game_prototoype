#include "Game.h"
#include "BdfFont.h"
#include "Physics.h"
#include "Map.h"
#include "Animation.h"
#include "Characters.h"

#define PI 3.14159
#define HALF_PI PI / 2.

const int CENTERX = TARGET_WIDTH / 2;
const int CENTERY = TARGET_HEIGHT / 2;

SDL_Event event;





int main(int argc, char ** argv)
{
    game = new Game();
    game->init();
    
    IMG_Init(IMG_INIT_PNG);

    // id -> shape
    // id is shifted by one. zero is air
    vector<int> tile_shapes = {
        TILE_SHAPE_NONE,
        TILE_SHAPE_CUBE,
        TILE_SHAPE_RAMP_L,
        TILE_SHAPE_RAMP_N,
        TILE_SHAPE_RAMP_R,
        TILE_SHAPE_RAMP_F,
        TILE_SHAPE_RAMP_LN,
        TILE_SHAPE_RAMP_NR,
        TILE_SHAPE_RAMP_RF,
        TILE_SHAPE_RAMP_FL
    };

    vector<vector<vector<int>>> map_data = {
        {
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1},
        },
        {
            {0, 0, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 5, 0, 0, 0, 1, 1},
            {0, 2, 1, 4, 0, 0, 3, 3},
            {0, 0, 3, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
        },
        {
            {1, 0, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 3, 3},
            {0, 0, 1, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
        },
        {
            {1, 0, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 3, 3},
            {0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 1, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0},
        },
    };




    map_data = {
        {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
        },
        {
            {1, 4, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1},
            {1, 4, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1},
            {1, 4, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1},
            {1, 4, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
            {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3},
            {0, 1, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        },
        {
            {1, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0},
            {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        },
        {
            {1, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 1, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0},
            {0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1},
            {0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1}
        },
        {
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 8},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 7},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        }
    };
    //map_data = {{{1}}};

    PhysicsSolver solver("");
        

    // x z y
    ivec3 tile_size = ivec3(17, 11, 9);
    ivec3 tile_offsets = ivec3(1, 1, 1);
    Atlas atlas = Atlas("assets/better_tiles.png", tile_size, tile_offsets);

    //atlas.generate_state_table();

    //return 0;

    // 0 1 2 3 4
    // 1 2 4


    Map map = Map(map_data, &atlas);

    map.render(&atlas);

    vec3 map_size = map.get_map_size();
    for (int y = 0; y < map_size.y; y++){
        for (int z = 0; z < map_size.z; z++){
            for (int x = map_size.x - 1; x >= 0; x--){
                int tile = map_data[y][z][x];
                int n = map.get_tile_neighbors_state({x, y, z});
                if (tile != 0){
                    
                    //*     Y
                    //*     |
                    //*     * -- X
                    //*      \
                    //*        Z
                    // todo: check solidness!
                    solver.add_tile({x,y,z}, tile_shapes[tile], n);

                }
            }
        }
    }


    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 frag;
        uniform vec2 size;
        uniform vec2 target_size;
        
        mat4 bayerIndex = mat4(
            vec4(00.0/16.0, 12.0/16.0, 03.0/16.0, 15.0/16.0),
            vec4(08.0/16.0, 04.0/16.0, 11.0/16.0, 07.0/16.0),
            vec4(02.0/16.0, 14.0/16.0, 01.0/16.0, 13.0/16.0),
            vec4(10.0/16.0, 06.0/16.0, 09.0/16.0, 05.0/16.0));
        
        float dither8x8(float r) {    
            float threshold = bayerIndex[int(gl_FragCoord.x)%4][int(gl_FragCoord.y)%4];
            //return mix(r, r * floor(floor(r * 64.) / 64. + threshold) * 1.5, 1);
            return r > threshold ? 1:0;
        }

        void main() {
            vec3 color1 = vec3(0.7, 0.94, 0.99);
            color1 = vec3(0.102, 0.6, 0.713) * 0.7;
            vec3 color2 = vec3(0.04, 0.35, 0.45) * 0.7;
            vec2 uv = gl_FragCoord.xy / size.xy;
            float y = clamp((gl_FragCoord.y - size.y / 4.) / 32 - cos((gl_FragCoord.x-size.x/2.)/size.x * 4.) * 0.5, 0, 1);
            frag = vec4(mix(color1, color2, dither8x8(y)), 1.);

            //if (pixelPos.x < big.x && pixelPos.x > sml.x && pixelPos.y < big.y && pixelPos.y > sml.y) FragColor.b = 1.;

            //frag = pixelPos.y > target_size.y / 2 ? vec4(0.7, 0.94, 0.99, 1.) : vec4(0.04, 0.35, 0.45, 1.);
        }
    )";

    GLint success;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Compute shader compilation failed: " << infoLog << std::endl;
    }


    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Compute shader program linking failed: " << infoLog << std::endl;
    }
    
    BDFAtlas font_atlas = BDFAtlas("assets/fonts/orp/orp-italic.bdf", 1536);
    game->debugger.init(&font_atlas);
    
    PhysicsPrimitive player = solver.capsule(1.5, 0.5);
    player.type = TYPE_RIGID;
    player.bounciness = 0.00001;
    player.mass = 1.;
    player.position.x = 3;
    player.position.y = 1;
    player.position.z = 3;
    player.friction = 0.5;
    solver.push(&player);

    bool key_a, key_d, key_w, key_s, key_space, key_shift;
    key_a = false;
    key_d = false;
    key_w = false;
    key_s = false;
    key_space = false;
    key_shift = false;

    game->debugger.register_line("vel","velocity: ","0, 0");
    game->debugger.register_line("inp","inp: ","0, 0");
    game->debugger.register_line("pos", "pos:  ", "0 0 0");
    game->debugger.register_line("phys", "collide:  ", "");
    game->debugger.register_line("phys0", "normal:  ", "");
    game->debugger.register_line("phys1", "phys:  ", "");
    game->debugger.register_line("phys2", "phys:  ", "");

    Animation run(".\\assets\\animations\\SPE_Run.json");
    Animation idle(".\\assets\\animations\\F.json");

    BloodKnight knight;

    vec2 shader_texture_size = ivec2(48, 48);
    //shader_texture_size = ivec2(128, 128);

    SDL_Surface* surface = IMG_Load("./assets/images/test.png");
    SDL_LockSurface(surface);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("./assets/images/knight.png", &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture" << std::endl;
        return -1;
    }
    ivec3 tex_size = ivec3(24, 24, 4);
    knight.shader.init(shader_texture_size.x, shader_texture_size.y, tex_size, data);
    stbi_image_free(data);
    
    map.render(&atlas);
    float anim_tick = 0;
    bool playing = true;
    bool forced_loop = true;
    vec2 last_dir = vec2(0, 1);
    while (game->is_running())
    {
        float time = game->time();
        while (SDL_PollEvent(&event)) {
            game->handle_event(event);
            if (event.type == SDL_KEYDOWN){
                switch (event.key.keysym.sym) {
                    case SDLK_a:
                        key_a = true;
                        break;
                    case SDLK_d:
                        key_d = true;
                        break;
                    case SDLK_w:
                        key_w = true;
                        break;
                    case SDLK_s:
                        key_s = true;
                        break;
                    case SDLK_SPACE:
                        key_space = true;
                        break;
                    case SDLK_LSHIFT:
                        key_shift = true;
                        break;
                }
            }
            if (event.type == SDL_KEYUP){
                switch (event.key.keysym.sym) {
                    case SDLK_a:
                        key_a = false;
                        break;
                    case SDLK_d:
                        key_d = false;
                        break;
                    case SDLK_w:
                        key_w = false;
                        break;
                    case SDLK_s:
                        key_s = false;
                        break;
                    case SDLK_SPACE:
                        key_space = false;
                        break;
                    case SDLK_LSHIFT:
                        key_shift = false;
                        break;
                }
            }
        }

        float delta = game->wrapped_delta();
        
        vec2 target_vel = vec2(key_d - key_a,  key_s - key_w) * 6.0f;

        if (playing){ // todo: integrate to Animation class
            anim_tick += (delta * 20);
            if (forced_loop){
                if (anim_tick > run.end_tick) anim_tick = run.return_tick + (int)anim_tick % (run.end_tick - run.return_tick);
            } else {
                anim_tick = glm::clamp((float)anim_tick, (float)run.start_tick, (float)run.end_tick);
            }
        }

        RawPose p;
        if (target_vel != vec2(0)){
            last_dir = target_vel;
            p = run.get_pose(anim_tick);
        }
        float angle = -atan2(last_dir.y, last_dir.x);
        p.torso_rot.y = angle * (180.0 / M_PI) + 90;
        knight.apply_pose(compute_pose(p, 0));

        //cout << player.velocity.z << endl;
        player.velocity.z = target_vel.y;
        player.velocity.x = target_vel.x;
        player.velocity.y = (key_space - key_shift) * 6.0f;

        solver.step(delta, nullptr);

        game->debugger.update_line("vel", to_string(player.velocity.x) + " " + to_string(player.velocity.y) + " " + to_string(player.velocity.z));
        game->debugger.update_line("inp", to_string(target_vel.x) + " " + to_string(target_vel.y));
        game->debugger.register_line("anim_tick", "anim_tick", to_string(anim_tick));
        game->debugger.update_line("pos", to_string(player.position.x) + " " + to_string(player.position.y) + " " + to_string(player.position.z));

        game->begin_main();
        
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform2f(glGetUniformLocation(shaderProgram, "target_size"), TARGET_WIDTH, TARGET_HEIGHT);
        glUniform2f(glGetUniformLocation(shaderProgram, "size"), game->get_screen_size().x, game->get_screen_size().y);
        game->draw_fullscreen_quad();
        glUseProgram(0);

        map.offset = (game->screen_pixel_size - map.get_texture_size());
        map.offset /= 2;
        glEnable(GL_BLEND);
        map.draw(game->screen_pixel_size);
        glDisable(GL_BLEND);
        game->end_main();

        game->begin_main();

        



        knight.shader.check_file_updates();
        knight.shader.use();
        // x+
        knight.shader.set_1f("time", time);
        knight.shader.set_2f("texture_size", shader_texture_size.x, shader_texture_size.y);
        
        knight.time_step(delta);


        //shader.set_position({cos(time) * 4, sin(time * 2) * 2, sin(time) * 4 + 4});

        knight.shader.set_position(player.position);

        knight.shader.update_map(map.get_depth(), map.get_texture_size(), map.get_map_size());
        glEnable(GL_BLEND);
        knight.shader.draw(game->screen_pixel_size);
        glDisable(GL_BLEND);


        game->debugger.update_basic();
        game->debugger.draw(game->screen_pixel_size);

        game->end_main();

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        game->draw_main();

        SDL_GL_SwapWindow(window);
        // todo: alpha checks for depth buffer draw :D
    }
    glDeleteProgram(shaderProgram);
    IMG_Quit();
    game->destroy();
    return 0;
}