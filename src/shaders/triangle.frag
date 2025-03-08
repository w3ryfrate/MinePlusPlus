#version 330 core

in vec3 fCol;
out vec4 Col;

void main() 
{
	Col = vec4(fCol, 1.0);
}