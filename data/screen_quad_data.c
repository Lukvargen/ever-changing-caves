#ifndef __SCREEN_QUAD_DATA__
#define __SCREEN_QUAD_DATA__

// Vertex data for quad
float screen_v_data[] = {
    -1.0f, -1.0f,  0.0f, 0.0f,  // Top Left
     1.0f, -1.0f,  1.0f, 0.0f,  // Top Right 
    -1.0f,  1.0f,  0.0f, 1.0f,  // Bottom Left
     1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
    /*
    0.0f, 0.0f, // Top Left
    1.0f, 0.0f, // Top Right
    0.0f, 1.0f, // Bottom Left
    1.0f, 1.0f  // Bottom Right
    */
};

int screen_i_data[] = {
    0, 3, 2,
    0, 1, 3
};

#ifdef GS_PLATFORM_WEB
    #define GS_VERSION_STR "#version 300 es\n"
#else
    #define GS_VERSION_STR "#version 330 core\n"
#endif

const char* screen_v_src =
GS_VERSION_STR
"layout(location = 0) in vec2 a_pos;\n"
"layout(location = 1) in vec2 a_uv;\n"
"precision mediump float;\n"
"uniform int u_shake;\n"
"uniform float u_time;\n"
"out vec2 uv;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(a_pos, 0.0, 1.0);\n"
"   uv = a_uv;\n"
"   if (u_shake != 0) {\n"
"       float strength = 0.01;\n"
"       gl_Position.x += cos(u_time * 10.0) * strength;\n"
"       gl_Position.y += cos(u_time * 15.0) * strength;\n"
"   }\n"
"}";

const char* screen_f_src =
GS_VERSION_STR
"precision mediump float;\n"
"uniform sampler2D u_tex;\n"
"uniform vec2 u_res;\n"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
/*
"const float offsets[9][2] = {\n"
"   {-1, 1},\n"
"   {-1, 1},\n"
"   {-1, 1},\n"
"   {-1, 1},\n"
"   {-1, 1},\n"
"   {-1, 1},\n"
"   {-1, 1},\n"
"   {-1, 1},\n"
"   {-1, 1},\n"
"};\n"
*/
"float vignette(vec2 _uv){\n"
"   _uv *= 1.0 - _uv.xy;\n"
"   float vignette = _uv.x * _uv.y * 15.0;\n"
"   return pow(vignette, 0.4 * 0.5);\n"
"}\n"

"void main()\n"
"{\n"
//"   frag_color = vec4(uv.x, uv.y, 1.0, 1.0);\n"
"   vec2 text_uv = uv;\n"
"   vec4 color = texture(u_tex, uv);\n"
"   text_uv = ceil(uv*u_res)/u_res;\n" // verkar som texturen blir upscalad / loopar inte igenom pixlar på bilden utan på alla pixlar som den tar upp på skärmen




// aberration
"   color.r = texture(u_tex, text_uv + vec2(0.0005, 0.0)).r;\n"
"   color.g = texture(u_tex, text_uv - vec2(0.0005, 0.0)).g;\n"
"   color.b = texture(u_tex, text_uv).b;\n"

"   color.rgb *= vignette(uv);\n"


"   color.r = clamp(color.r * 1.1, 0.0, 1.0);\n"
"   color.g = clamp(color.g * 1.1, 0.0, 1.0);\n"
"   color.b = clamp(color.b * 1.1, 0.0, 1.0);\n"

"   float saturation = 0.1;\n"
"   float contrast = 1.2;\n"
"   vec3 greyscale = vec3(color.r + color.g + color.b)/3.0;\n"
"   color.rgb = mix(color.rgb, greyscale, saturation);\n"
"   float midpoint = pow(0.5, 2.2);\n"
"   color.rgb = (color.rgb - vec3(midpoint)) * contrast + midpoint;\n"
"   frag_color = color;\n"
"}";

#endif