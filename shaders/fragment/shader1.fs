#version 330 core

out vec4 FragColor;

in vec3 normal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform float datacol;

void main()
{
    vec3 lightDir = vec3(0.4f, 0.6f, 1.0f);
    lightDir = normalize(lightDir);
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 albedo = vec3(0.8, 0.7, 0.4);
    float diff = abs(dot(norm, lightDir));
    float ambient = 0.2;
    float lighting = min(diff + ambient, 1.0);

    FragColor = vec4(albedo*lighting, 1.0);
}