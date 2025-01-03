#version 430 core

out vec4 FragColor;

in vec3 Normal;

uniform vec3 sunDirection;
uniform vec3 color;
uniform vec3 ambient;

void main(){
    float f = max(dot(-sunDirection, normalize(Normal)), 0.0);
    vec3 c = ambient + color * f;

    c = pow(c, vec3(1.0 / 2.2)); 

    FragColor = vec4(c,1.0);
}