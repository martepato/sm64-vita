#ifdef TARGET_VITA

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef _LANGUAGE_C
#define _LANGUAGE_C
#endif
#include <PR/gbi.h>

#include <vitaGL.h>

#include <psp2/io/fcntl.h>

#include "gfx_cc.h"
#include "gfx_rendering_api.h"

struct ShaderProgram {
    uint32_t shader_id;
    GLuint opengl_program_id;
    uint8_t num_inputs;
    bool used_textures[2];
    uint8_t num_floats;
    GLint attrib_locations[7];
    uint8_t attrib_sizes[7];
    uint8_t num_attribs;
};

static struct ShaderProgram shader_program_pool[64];
static uint8_t shader_program_pool_size;

static uint16_t *vgl_indices = NULL;
static struct ShaderProgram *cur_shader = NULL;

static bool gfx_vitagl_z_is_from_0_to_1(void) {
    return false;
}

static void gfx_vitagl_unload_shader(struct ShaderProgram *old_prg) {
    if (!old_prg || old_prg == cur_shader)
        cur_shader = NULL;
}

static void gfx_vitagl_load_shader(struct ShaderProgram *new_prg) {
    glUseProgram(new_prg->opengl_program_id);

    cur_shader = new_prg;
}

static void append_str(char *buf, size_t *len, const char *str) {
    while (*str != '\0')
        buf[(*len)++] = *str++;
}

static void append_line(char *buf, size_t *len, const char *str) {
    while (*str != '\0')
        buf[(*len)++] = *str++;
    buf[(*len)++] = '\n';
}

static const char *shader_item_to_str(uint32_t item, bool with_alpha, bool only_alpha,
                                      bool inputs_have_alpha, bool hint_single_element) {
    if (!only_alpha) {
        switch (item) {
            case SHADER_0:
                return with_alpha ? "float4(0.0, 0.0, 0.0, 0.0)" : "float3(0.0, 0.0, 0.0)";
            case SHADER_INPUT_1:
                return with_alpha || !inputs_have_alpha ? "vInput1" : "vInput1.rgb";
            case SHADER_INPUT_2:
                return with_alpha || !inputs_have_alpha ? "vInput2" : "vInput2.rgb";
            case SHADER_INPUT_3:
                return with_alpha || !inputs_have_alpha ? "vInput3" : "vInput3.rgb";
            case SHADER_INPUT_4:
                return with_alpha || !inputs_have_alpha ? "vInput4" : "vInput4.rgb";
            case SHADER_TEXEL0:
                return with_alpha ? "texVal0" : "texVal0.rgb";
            case SHADER_TEXEL0A:
                return hint_single_element
                           ? "texVal0.a"
                           : (with_alpha ? "float4(texelVal0.a, texelVal0.a, texelVal0.a, texelVal0.a)"
                                         : "float3(texelVal0.a, texelVal0.a, texelVal0.a)");
            case SHADER_TEXEL1:
                return with_alpha ? "texVal1" : "texVal1.rgb";
        }
    } else {
        switch (item) {
            case SHADER_0:
                return "0.0";
            case SHADER_INPUT_1:
                return "vInput1.a";
            case SHADER_INPUT_2:
                return "vInput2.a";
            case SHADER_INPUT_3:
                return "vInput3.a";
            case SHADER_INPUT_4:
                return "vInput4.a";
            case SHADER_TEXEL0:
                return "texVal0.a";
            case SHADER_TEXEL0A:
                return "texVal0.a";
            case SHADER_TEXEL1:
                return "texVal1.a";
        }
    }
}

static void append_formula(char *buf, size_t *len, uint8_t c[2][4], bool do_single, bool do_multiply,
                           bool do_mix, bool with_alpha, bool only_alpha, bool opt_alpha) {
    if (do_single) {
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][3], with_alpha, only_alpha, opt_alpha, false));
    } else if (do_multiply) {
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, " * ");
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
    } else if (do_mix) {
        append_str(buf, len, "lerp(");
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][1], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, ", ");
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, ", ");
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
        append_str(buf, len, ")");
    } else {
        append_str(buf, len, "(");
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][0], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, " - ");
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][1], with_alpha, only_alpha, opt_alpha, false));
        append_str(buf, len, ") * ");
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][2], with_alpha, only_alpha, opt_alpha, true));
        append_str(buf, len, " + ");
        append_str(buf, len,
                   shader_item_to_str(c[only_alpha][3], with_alpha, only_alpha, opt_alpha, false));
    }
}

static struct ShaderProgram *gfx_vitagl_create_and_load_new_shader(uint32_t shader_id) {
    uint8_t c[2][4];
    for (int i = 0; i < 4; i++) {
        c[0][i] = (shader_id >> (i * 3)) & 7;
        c[1][i] = (shader_id >> (12 + i * 3)) & 7;
    }
    bool opt_alpha = (shader_id & SHADER_OPT_ALPHA) != 0;
    bool opt_fog = (shader_id & SHADER_OPT_FOG) != 0;
    bool opt_texture_edge = (shader_id & SHADER_OPT_TEXTURE_EDGE) != 0;
    bool used_textures[2] = { 0, 0 };
    int num_inputs = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            if (c[i][j] >= SHADER_INPUT_1 && c[i][j] <= SHADER_INPUT_4) {
                if (c[i][j] > num_inputs) {
                    num_inputs = c[i][j];
                }
            }
            if (c[i][j] == SHADER_TEXEL0 || c[i][j] == SHADER_TEXEL0A) {
                used_textures[0] = true;
            }
            if (c[i][j] == SHADER_TEXEL1) {
                used_textures[1] = true;
            }
        }
    }
    bool do_single[2] = { c[0][2] == 0, c[1][2] == 0 };
    bool do_multiply[2] = { c[0][1] == 0 && c[0][3] == 0, c[1][1] == 0 && c[1][3] == 0 };
    bool do_mix[2] = { c[0][1] == c[0][3], c[1][1] == c[1][3] };
    bool color_alpha_same = (shader_id & 0xfff) == ((shader_id >> 12) & 0xfff);

    char vs_buf[1024];
    char fs_buf[1024];
    size_t vs_len = 0;
    size_t fs_len = 0;
    size_t num_floats = 4;

    bool has_texture = false;
    bool has_fog = false;

    if (used_textures[0] || used_textures[1]) {
        num_floats += 2;
        has_texture = true;
    }
    if (opt_fog) {
        num_floats += 4;
        has_fog = true;
    }
    for (int i = 0; i < num_inputs; i++) {
        num_floats += opt_alpha ? 4 : 3;
    }

    char filename[64];

    sprintf(filename, "app0:shaders/%08X_v.cg", shader_id);

    FILE *fd = fopen(filename, "rb");
    fseek(fd, 0, SEEK_END);
    vs_len = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    fread(vs_buf, vs_len, 1, fd);
    fclose(fd);
    vs_buf[vs_len] = '\0';

    sprintf(filename, "app0:shaders/%08X_f.cg", shader_id);

    fd = fopen(filename, "rb");
    fseek(fd, 0, SEEK_END);
    fs_len = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    fread(fs_buf, fs_len, 1, fd);
    fclose(fd);
    fs_buf[fs_len] = '\0';

    const GLchar *sources[2] = { vs_buf, fs_buf };
    const GLint lengths[2] = { vs_len, fs_len };
    GLint success;

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &sources[0], &lengths[0]);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &sources[1], &lengths[1]);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    int offset = 0;
    vglBindPackedAttribLocation(shader_program, "aVtxPos", 4, GL_FLOAT, offset,
                                num_floats * sizeof(float));
    offset += 4 * sizeof(float);

    if (has_texture) {
        vglBindPackedAttribLocation(shader_program, "aTexCoord", 2, GL_FLOAT, offset,
                                    num_floats * sizeof(float));
        offset += 2 * sizeof(float);
    }
    if (has_fog) {
        vglBindPackedAttribLocation(shader_program, "aFog", 4, GL_FLOAT, offset,
                                    num_floats * sizeof(float));
        offset += 4 * sizeof(float);
    }
    char name[8];
    for (int i = 0; i < num_inputs; i++) {
        snprintf(name, 8, "aInput%d", i + 1);
        vglBindPackedAttribLocation(shader_program, name, opt_alpha ? 4 : 3, GL_FLOAT, offset,
                                    num_floats * sizeof(float));
        offset += sizeof(float) * (opt_alpha ? 4 : 3);
    }

    glLinkProgram(shader_program);

    size_t cnt = 0;

    struct ShaderProgram *prg = &shader_program_pool[shader_program_pool_size++];

    prg->shader_id = shader_id;
    prg->opengl_program_id = shader_program;
    prg->num_inputs = num_inputs;
    prg->used_textures[0] = used_textures[0];
    prg->used_textures[1] = used_textures[1];
    prg->num_floats = num_floats;
    prg->num_attribs = cnt;

    gfx_vitagl_load_shader(prg);

    return prg;
}

static struct ShaderProgram *gfx_vitagl_lookup_shader(uint32_t shader_id) {
    for (size_t i = 0; i < shader_program_pool_size; i++) {
        if (shader_program_pool[i].shader_id == shader_id) {
            return &shader_program_pool[i];
        }
    }
    return NULL;
}

static void gfx_vitagl_shader_get_info(struct ShaderProgram *prg, uint8_t *num_inputs,
                                       bool used_textures[2]) {
    *num_inputs = prg->num_inputs;
    used_textures[0] = prg->used_textures[0];
    used_textures[1] = prg->used_textures[1];
}

static GLuint gfx_vitagl_new_texture(void) {
    GLuint ret;
    glGenTextures(1, &ret);
    return ret;
}

static void gfx_vitagl_select_texture(int tile, GLuint texture_id) {
    glActiveTexture(GL_TEXTURE0 + tile);
    glBindTexture(GL_TEXTURE_2D, texture_id);
}

static void gfx_vitagl_upload_texture(uint8_t *rgba32_buf, int width, int height) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba32_buf);
}

static uint32_t gfx_cm_to_opengl(uint32_t val) {
    if (val & G_TX_CLAMP) {
        return GL_CLAMP_TO_EDGE;
    }
    return (val & G_TX_MIRROR) ? GL_MIRROR_CLAMP_EXT : GL_REPEAT;
}

static void gfx_vitagl_set_sampler_parameters(int tile, bool linear_filter, uint32_t cms,
                                              uint32_t cmt) {
    glActiveTexture(GL_TEXTURE0 + tile);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear_filter ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear_filter ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gfx_cm_to_opengl(cms));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gfx_cm_to_opengl(cmt));
}

static void gfx_vitagl_set_depth_test(bool depth_test) {
    if (depth_test) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

static void gfx_vitagl_set_depth_mask(bool z_upd) {
    glDepthMask(z_upd ? GL_TRUE : GL_FALSE);
}

static void gfx_vitagl_set_zmode_decal(bool zmode_decal) {
    if (zmode_decal) {
        glPolygonOffset(-2, -2);
        glEnable(GL_POLYGON_OFFSET_FILL);
    } else {
        glPolygonOffset(0, 0);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

static void gfx_vitagl_set_viewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

static void gfx_vitagl_set_scissor(int x, int y, int width, int height) {
    glScissor(x, y, width, height);
}

static void gfx_vitagl_set_use_alpha(bool use_alpha) {
    if (use_alpha) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}

static void gfx_vitagl_draw_triangles(float buf_vbo[], size_t buf_vbo_len, size_t buf_vbo_num_tris) {
    const size_t count = buf_vbo_num_tris * 3;

    vglVertexAttribPointer(0, cur_shader->num_floats, GL_FLOAT, GL_FALSE, 0, count, buf_vbo);
    vglDrawObjects(GL_TRIANGLES, count, false);
}

static void gfx_vitagl_init(void) {
    // generate an index buffer
    // vertices that we get are always sequential, so it will stay constant
    vgl_indices = malloc(sizeof(uint16_t) * 8192);
    for (uint16_t i = 0; i < 8192; ++i)
        vgl_indices[i] = i;

    vglEnableRuntimeShaderCompiler(GL_TRUE);
    vglUseVram(GL_TRUE);
    vglWaitVblankStart(GL_TRUE);

    vglInitExtended(0x800000, 960, 544, 0x1000000, SCE_GXM_MULTISAMPLE_4X);

    vglIndexPointerMapped(vgl_indices);

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void gfx_vitagl_on_resize() {
}

static void gfx_vitagl_start_frame(void) {
    vglStartRendering();

    glDisable(GL_SCISSOR_TEST);
    glDepthMask(GL_TRUE); // Must be set to clear Z-buffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
}

static void gfx_vitagl_end_frame() {
}

static void gfx_vitagl_finish_render() {
}

struct GfxRenderingAPI gfx_vitagl_api = {
    gfx_vitagl_z_is_from_0_to_1,
    gfx_vitagl_unload_shader,
    gfx_vitagl_load_shader,
    gfx_vitagl_create_and_load_new_shader,
    gfx_vitagl_lookup_shader,
    gfx_vitagl_shader_get_info,
    gfx_vitagl_new_texture,
    gfx_vitagl_select_texture,
    gfx_vitagl_upload_texture,
    gfx_vitagl_set_sampler_parameters,
    gfx_vitagl_set_depth_test,
    gfx_vitagl_set_depth_mask,
    gfx_vitagl_set_zmode_decal,
    gfx_vitagl_set_viewport,
    gfx_vitagl_set_scissor,
    gfx_vitagl_set_use_alpha,
    gfx_vitagl_draw_triangles,
    gfx_vitagl_init,
    gfx_vitagl_on_resize,
    gfx_vitagl_start_frame,
    gfx_vitagl_end_frame,
    gfx_vitagl_finish_render
};

#endif
