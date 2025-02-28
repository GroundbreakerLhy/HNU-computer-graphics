#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec3 vertexColor;

uniform mat4 modelTransformMatrix;

out vec3 theColor;

void main()
{
	vec4 v = vec4(position, 1.0);
	vec4 out_position = modelTransformMatrix * v;
	gl_Position = out_position;	
	theColor = vertexColor;
}
