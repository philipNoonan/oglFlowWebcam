#version 430 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec2 flow;
} gs_in[];

const float MAGNITUDE = 1.0;

void GenerateLine(int index)
{
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].flow.x / 1920.0f, gs_in[index].flow.y / 1080.0f, 0.0, 0.0) * MAGNITUDE;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0);

}