#version 330 core

in vec2 UV;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform sampler2D textureSampler;
uniform vec3 viewPos;

// 光照参数
uniform vec3 dirLight_direction;
uniform vec3 dirLight_ambient;
uniform vec3 dirLight_diffuse;
uniform vec3 dirLight_specular;
uniform float dirLight_intensity;

uniform vec3 pointLight_position;
uniform vec3 pointLight_ambient;
uniform vec3 pointLight_diffuse;
uniform vec3 pointLight_specular;
uniform float pointLight_constant;
uniform float pointLight_linear;
uniform float pointLight_quadratic;

void main()
{
    vec3 norm = normalize(Normal);
    
    // 方向光计算
    vec3 lightDir = normalize(-dirLight_direction);
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    vec3 ambient = dirLight_ambient * dirLight_intensity;
    vec3 diffuse = dirLight_diffuse * diff * dirLight_intensity;
    vec3 specular = dirLight_specular * spec * dirLight_intensity;
    vec3 dirResult = ambient + diffuse + specular;
    
    // 点光源计算
    lightDir = normalize(pointLight_position - FragPos);
    diff = max(dot(norm, lightDir), 0.0);
    reflectDir = reflect(-lightDir, norm);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    
    float distance = length(pointLight_position - FragPos);
    float attenuation = 1.0 / (pointLight_constant + pointLight_linear * distance + 
                     pointLight_quadratic * (distance * distance));
    
    vec3 pointAmbient = pointLight_ambient * attenuation;
    vec3 pointDiffuse = pointLight_diffuse * diff * attenuation;
    vec3 pointSpecular = pointLight_specular * spec * attenuation;
    vec3 pointResult = pointAmbient + pointDiffuse + pointSpecular;
    
    vec3 result = texture(textureSampler, UV).rgb * (dirResult + pointResult);
    color = vec4(result, 1.0);
}
