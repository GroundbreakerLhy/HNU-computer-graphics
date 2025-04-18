#version 330 core
in vec2 TexCoords;
in float Alpha;

out vec4 color;

void main()
{
    float dist = length(TexCoords - vec2(0.5));
    if(dist > 0.5) discard;
    
    float intensity = 1.0 - 2.0 * dist;
    color = vec4(1.0, 1.0, 1.0, Alpha * intensity * 0.7);
}
