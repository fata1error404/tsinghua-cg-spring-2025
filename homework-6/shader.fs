#version 330 core

in vec3 Position;
in vec3 Normal;

out vec4 FragColor;

struct Light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float shininess;
};
  
uniform Material material;

uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform vec3 objectColor;
uniform bool isAmbientOn;
uniform bool isDiffuseOn;
uniform bool isSpecularOn;

void main()
{
    vec3 ambient = material.ambient * light.ambient;

    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - Position);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = material.diffuse * diff * light.diffuse;

    vec3 viewDir = normalize(cameraPos - Position);
    vec3 reflectDir = reflect(-L, N);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = material.specular * spec * light.specular;  

    vec3 result = ((isAmbientOn  ? ambient  : vec3(0.0)) + (isDiffuseOn  ? diffuse  : vec3(0.0)) + (isSpecularOn  ? specular  : vec3(0.0))) * objectColor;
    FragColor = vec4(result, 1.0);
}