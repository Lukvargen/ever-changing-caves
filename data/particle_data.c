
// Vertex data for quad
float quad_v_data[] = {
//  POSITION    UV
    -0.5f, -0.5f, 0.0f, 0.0f, // Top Left 
    0.5f, -0.5f, 1.0f, 0.0f, // Top Right
    -0.5f, 0.5f, 0.0f, 1.0f, // Bottom Left
    0.5f, 0.5f, 1.0f, 1.0f  // Bottom Right
};

int quad_i_data[] = {
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
"layout(location = 1) in vec2 a_uv;\n"
"precision mediump float;\n"
"uniform mat4 u_mvp;\n"
"out vec2 uv;\n"
"void main()\n"
"{\n"
"   gl_Position = u_mvp * vec4(a_pos, 0.0, 1.0);\n"
"   uv = a_uv;\n"
"}";

const char* particle_f_src =
GS_VERSION_STR
"precision mediump float;\n"
"uniform vec4 u_color;\n"
"uniform sampler2D u_tex;\n"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   vec4 color = u_color;\n"
"   if (texture(u_tex, uv).a == 0.0) {\n"
"       color.a = 0.0;\n"
"   }\n"
"   frag_color = color;\n"
"}";
