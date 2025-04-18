#version 330 core
layout(location = 0) in vec3 position;

out vec2 TexCoords;
out float Alpha;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float life;

void main()
{
    TexCoords = position.xy + vec2(0.5);
    Alpha = life;
    
    mat4 modelView = view * model;
    
    modelView[0][0] = 1.0;
    modelView[0][1] = 0.0;
    modelView[0][2] = 0.0;
    
    modelView[1][0] = 0.0;
    modelView[1][1] = 1.0;
    modelView[1][2] = 0.0;
    
    modelView[2][0] = 0.0;
    modelView[2][1] = 0.0;
    modelView[2][2] = 1.0;
    
    gl_Position = projection * modelView * vec4(position, 1.0);
}
