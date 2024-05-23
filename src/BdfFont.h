#include "Game.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
using namespace std;
#include <glm/glm.hpp>
using namespace glm;

#ifndef MY_BDF
#define MY_BDF

struct BDFChar{
    int encoding;
    int startx;
    int endx;
    vector<int> bitmap;
};

struct BDFFont{
    int height;
    vector<BDFChar> chars;
};


class BDFAtlas{
    double remap(double value, double fromLow, double fromHigh, double toLow, double toHigh) {
        double normalized = (value - fromLow) / (fromHigh - fromLow);
        return toLow + normalized * (toHigh - toLow);
    }

    vec2 remap_vec2(vec2 value, vec2 fromLow, vec2 fromHigh, vec2 toLow, vec2 toHigh) {
        vec2 normalized = (value - fromLow) / (fromHigh - fromLow);
        return toLow + normalized * (toHigh - toLow);
    }

    BDFChar read_bdf_char(ifstream* file){
        BDFChar out_char = BDFChar{};
        string line;
        bool reading_bitmap = false;
        int first = -1;
        int last = 15;
        while (getline(*file, line)) {
            if (line.find("ENCODING") != string::npos){
                istringstream iss(line);
                string token;
                iss >> token;
                iss >> token;
                out_char.encoding = stoi(token);
                continue;
            }
            if (line.find("ENDCHAR") != string::npos) {
                out_char.startx = first;
                out_char.endx = last;
                break;
            }
            if (reading_bitmap){
                int value;
                stringstream ss;
                ss << hex << line;
                ss >> value;
                int line_first = -1;
                int line_last = 16;
                for (int i = 15; i >= 0; --i) {
                    if (((value >> i) & 1) != 0){
                        if (line_first == -1) line_first = i;
                        line_last = std::min(line_last, i);
                    }
                }
                first = std::max(first, line_first);
                if (line_last != 16) last = std::min(last, line_last);
                out_char.bitmap.push_back(value);
            }
            if (line.find("BITMAP") != string::npos) reading_bitmap = true;
        }
        return out_char;
    }

    BDFFont read_bdf_font(string path, int number_of_glyphs){
        ifstream file(path);
        BDFChar null_char = BDFChar{};
        font = BDFFont();
        if (!file.is_open()) {
            cerr << "Error: Unable to open file " << path << endl;
            return font;
        }
        string line;

        while (getline(file, line)) {
            if (line.find("STARTCHAR") != string::npos){
                BDFChar new_char = read_bdf_char(&file);
                if (new_char.encoding == 0) {
                    null_char = new_char;
                }
                for (int i = font.chars.size(); i < new_char.encoding; i++){
                    if (number_of_glyphs > 0 && font.chars.size() >= number_of_glyphs) break;
                    font.chars.push_back(null_char);
                    continue;
                }
                if (number_of_glyphs > 0 && font.chars.size() >= number_of_glyphs) break;
                font.chars.push_back(new_char);
            };
            if (line.find("FONTBOUNDINGBOX") != string::npos){
                istringstream iss(line);
                string token;
                iss >> token;
                iss >> token;
                glyph_size.x = stoi(token);
                iss >> token;
                glyph_size.y = stoi(token);
            };
        }
        return font;
    }
    vec2 get_glyph_pos(int i){
        return ivec2{
            i % atlas_width,
            std::floor((float)i / (float)atlas_width),
        } * glyph_size;
    }
    void render_font(){
        atlas_size_px = {
            glyph_size.x * atlas_width,
            glyph_size.y * (std::ceil((float)font.chars.size() / (float)atlas_width)),
        };
        if (atlas_size_px.y == 0) atlas_size_px.y = glyph_size.y;
        glGenTextures(1, &atlas_texture);
        glBindTexture(GL_TEXTURE_2D, atlas_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas_size_px.x, atlas_size_px.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenFramebuffers(1, &atlas_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, atlas_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, atlas_texture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, atlas_fbo);
        glViewport(0, 0, atlas_size_px.x, atlas_size_px.y);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        //glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_BLEND_COLOR);


        glPointSize(1);
        glBegin(GL_POINTS);
        glColor3f(1, 1, 1);
        for (int i = 0; i < font.chars.size(); i++){
            BDFChar glyph = font.chars[i];
            ivec2 offset = get_glyph_pos(i);
            /*for (int x = 1; x < glyph_size.x-1; x++)
            for (int y = 1; y < glyph_size.y-1; y++){
                ivec2 pos = offset + ivec2{x, y};
                vec2 uv_pos = remap_vec2(pos, {0, 0}, atlas_size_px, {-1, -1}, {1, 1});
                glColor3f(1, 1, 1); glVertex2f(uv_pos.x, uv_pos.y);
            }
            */
            for (int y = 0; y < glyph.bitmap.size(); y++){
                int line = glyph.bitmap[y];
                for (int x = glyph.startx; x >= glyph.endx; --x) {
                    if (((line >> x) & 1) != 0){
                        ivec2 pos = offset + ivec2{glyph.startx-x, y};
                        vec2 uv_pos = remap_vec2(pos, {0, 0}, atlas_size_px, {-1, -1}, {1, 1});
                        glColor3f(1, 1, 1); glVertex2f(uv_pos.x, uv_pos.y);
                    } else {
                        ivec2 pos = offset + ivec2{glyph.startx-x, y};
                        vec2 uv_pos = remap_vec2(pos, {0, 0}, atlas_size_px, {-1, -1}, {1, 1});
                        glColor3f(0, 0, 0); glVertex2f(uv_pos.x, uv_pos.y);
                    }
                }
            }
        }
        glColor3f(1, 1, 1);
        glEnd();
        

        /*glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex2f(-1, -1);
        glVertex2f(0.5, 1);
        glColor3f(1, 1, 1);
        glEnd();*/


        glDisable(GL_BLEND);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    GLuint atlas_texture = 0;
    GLuint atlas_fbo = 0;
    BDFFont font;
    const int atlas_width = 64;
    ivec2 glyph_size;
    ivec2 atlas_size_px;

    public:
    BDFAtlas(string path, int number_of_glyphs = 0){
        read_bdf_font(path, number_of_glyphs);
        render_font();
    }




    void draw_text(string text, vec2 dst, vec2 screen_size){
        int i = 0;
        for (wchar_t c : text){
            vec2 offset = {glyph_size.x * i, 0};
            draw_char(c, dst+offset, screen_size);
            i++;
        }
    }

    void draw_char(wchar_t c, vec2 dst, vec2 screen_size){
        int i = int(c);
        //BDFChar char_data = (i >= font.chars.size())?font.chars[0]:font.chars[i]; // todo: freeing after generating texture
        vec2 pos = get_glyph_pos(i);
        vec4 uv_src = vec4{
            remap_vec2(pos, {}, get_atlas_size(), {0, 0}, {1, 1}),
            remap_vec2(pos + get_glyph_size(), {}, get_atlas_size(), {0, 0}, {1, 1}),
        };
        
        vec4 uv_dst = vec4{
            remap_vec2(dst, {}, screen_size, {-1, -1}, {1, 1}),
            remap_vec2(dst + get_glyph_size(), {}, screen_size, {-1, -1}, {1, 1}),
        };
        glBindTexture(GL_TEXTURE_2D, get_texture());
        glBegin(GL_QUADS);
        glColor3f(1, 1, 1);
        glTexCoord2f(uv_src.x, uv_src.w); glVertex2f(uv_dst.x, uv_dst.y);
        glTexCoord2f(uv_src.z, uv_src.w); glVertex2f(uv_dst.z, uv_dst.y);
        glTexCoord2f(uv_src.z, uv_src.y); glVertex2f(uv_dst.z, uv_dst.w);
        glTexCoord2f(uv_src.x, uv_src.y); glVertex2f(uv_dst.x, uv_dst.w);
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    /*void save_texture(){
        glBindTexture(GL_TEXTURE_2D, get_texture());
        GLubyte* textureData = new GLubyte[atlas_size_px.x * atlas_size_px.y * 4];
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
        stbi_write_png("texture.png", get_atlas_size().x, get_atlas_size().y, 4, textureData, get_atlas_size().x * 4);
    }*/
    vec2 get_atlas_size() {return atlas_size_px;}
    vec2 get_glyph_size() {return glyph_size;}
    BDFFont get_font() {return font;}
    GLuint get_texture() {return atlas_texture;}
};
#endif