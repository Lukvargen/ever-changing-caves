#ifndef __PARTICLE_DATA__
#define __PARTICLE_DATA__

// Vertex data for quad
float particle_v_data[] = {
    0.0f, 0.0f, // Top Left
    1.0f, 0.0f, // Top Right
    0.0f, 1.0f, // Bottom Left
    1.0f, 1.0f  // Bottom Right
};

int particle_i_data[] = {
    0, 3, 2,
    0, 1, 3
};

#ifdef GS_PLATFORM_WEB
    #define GS_VERSION_STR "#version 300 es\n"
#else
    #define GS_VERSION_STR "#version 330 core\n"
#endif

const char* particle_v_src =
GS_VERSION_STR
"layout(location = 0) in vec2 a_pos;\n"
"precision mediump float;\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_projection;\n"
"out vec2 uv;\n"
"void main()\n"
"{\n"
"   gl_Position = u_projection * u_model * vec4(a_pos, 0.0, 1.0);\n"
"   uv = a_pos;\n"
"}";

const char* particle_f_src =
GS_VERSION_STR
"precision mediump float;\n"
"uniform vec3 u_color;\n"
"uniform vec2 u_res;\n"
"uniform sampler2D u_tex;\n"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   frag_color = vec4(u_color, 1.0);\n"
"}";

#endif