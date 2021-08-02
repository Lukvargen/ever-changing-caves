typedef struct tile_vertex_data_t
{
    gs_vec2 position;
    gs_vec3 color;
} tile_vertex_data_t;

tile_vertex_data_t tile_v_data[TILES_SIZE_X*TILES_SIZE_Y*4];

int tile_i_data[TILES_SIZE_X*TILES_SIZE_Y*6];

#ifdef GS_PLATFORM_WEB
    #define GS_VERSION_STR "#version 300 es\n"
#else
    #define GS_VERSION_STR "#version 330 core\n"
#endif

const char* tile_v_src =
GS_VERSION_STR
"layout(location = 0) in vec2 a_pos;\n"
"layout(location = 1) in vec3 a_color;\n"
"precision mediump float;\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_projection;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"   gl_Position = u_projection * u_model * vec4(a_pos, 0.0, 1.0);\n"
"   color = a_color;\n"
"}";

const char* tile_f_src =
GS_VERSION_STR
"precision mediump float;\n"
"in vec3 color;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   frag_color = vec4(color, 1.0);\n"
"}";

