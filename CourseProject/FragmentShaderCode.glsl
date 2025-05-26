#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoord;
in vec3 Normal;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
};

uniform Light light1;
uniform Light light2;
uniform vec3 viewPos;

uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;
uniform bool useNormalMap;

vec3 calculateLight(Light light, vec3 normal, vec3 fragPos, vec3 viewPos)
{
    // 环境光
    vec3 ambient = light.ambient * texture(diffuseTexture, TexCoord).rgb;
    
    // 漫反射
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(diffuseTexture, TexCoord).rgb;
    
    // 镜面反射
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = light.specular * spec;
    
    return (ambient + diffuse + specular) * light.intensity;
}

void main()
{
    vec3 normal = normalize(Normal);
    
    // 如果使用法线贴图
    if (useNormalMap) {
        // 简化的法线贴图实现
        vec3 normalMap = texture(normalMap, TexCoord).rgb;
        normalMap = normalize(normalMap * 2.0 - 1.0);
        
        // 为简化，直接使用法线贴图的值
        normal = normalize(Normal + normalMap * 0.3);
    }
    
    // 计算两个光源的光照
    vec3 result = calculateLight(light1, normal, FragPos, viewPos);
    result += calculateLight(light2, normal, FragPos, viewPos);
    
    FragColor = vec4(result, 1.0);
}
