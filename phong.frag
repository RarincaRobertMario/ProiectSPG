#version 330 core

/* --- START GENERAL IN VARIABLES --- */

    in vec3 FragPos;    // World-space position.
    in vec3 Normal;     // World-space normal.
    in vec2 TexCoords;  // Texture coordinates.
    in vec3 Tangents;   // Tangents
    in vec3 PassColor;  // Color

/* --- STOP GENERAL IN VARIABLES --- */

/* --- START GENERAL OUT VARIABLES --- */

    out vec4 FragColor; // Output fragment color.

/* --- STOP GENERAL OUT VARIABLES --- */

/* --- START SPOTLIGHT VARIABLES --- */

    #define MAX_SPOTLIGHTS 16    // Absolutely no idea what this is (I am lying).

    // Structure for a spot light (see SpotLight class in C++).
    struct SpotLight
    {
        /* LIGHT PROPERTIES */
        vec3 position;      // Position of the spotlight.
        vec3 color;         // Current color of the spotlight.

        float constant;     // Constant attenuation.
        float linear;       // Linear attenuation.
        float quadratic;    // Quadratic attenuation.

        float intensity;    // Intensity of the light.

        /* SPOTLIGHT PROPERTIES */
        vec3 direction;     // Direction of the spotlight.

        float cutOff;       // cos(inner angle).
        float outerCutOff;  // cos(outer angle).

        int shadowMapIdx;   // Shadow map index.
    };

    uniform SpotLight spotLights[MAX_SPOTLIGHTS];   // Vector of spotlights.
    uniform int spotLightCount;                     // Number of spotlights.

/* --- STOP SPOTLIGHT VARIABLES --- */

/* --- START POINTLIGHT VARIABLES --- */

    #define MAX_POINTLIGHTS 16

    // Structure for a point light (see PointLight class in C++)
    struct PointLight
    {
        /* LIGHT PROPERTIES */
        vec3 position;          // Position of the spotlight.
        vec3 color;             // Current color of the spotlight.

        float constant;         // Constant attenuation.
        float linear;           // Linear attenuation.
        float quadratic;        // Quadratic attenuation.

        float intensity;        // Intensity of the light.

        /* POINT LIGHT PROPERTIES */
        int shadowCubeIndex;    // Shadow Cube index. See ShadowCubeIndex and ShadowCube struct in C++
    };

    uniform PointLight pointLights[MAX_POINTLIGHTS];    // Vector of point lights.
    uniform int pointLightCount;                        // Number of point lights.

/* --- STOP POINTLIGHT VARIABLES --- */

/* --- START DIRLIGHT VARIABLES --- */

    // Directional light of infite direction (moon) -  see DirLight class in C++
    struct DirLight 
    {
        vec3 direction;     // Direction
        vec3 color;         // Color
        float intensity;    // Intensity
    };

    uniform DirLight moonLight; // Moon light

/* --- STOP DIRLIGHT VARIABLES --- */

/* --- START GENERAL UNIFORM VARIABLES --- */

    uniform vec3 viewPos;     // Camera position
    uniform int isLightMesh = 0;                // Flag if the vertex / mesh is a light and should be lit up regardless.
    uniform float lightMeshIntensity = 1.0f;    // Glow strength of the mesh (if a light mesh)
    uniform int fullBright = 0;     // If the scene should be rendered with full bright or not.

/* --- STOP GENERAL UNIFORM VARIABLES --- */

/* --- START TEXTURE VARIABLES --- */

    // TEXTURE_UNIT_BASE = 0
    uniform sampler2D texture_diffuse;  // Texture itself
    uniform int hasTexture = 0;         // Flag if we have a texture or not

/* --- STOP TEXTURE VARIABLES --- */

/* --- START SHADOW MAP VARIABLES --- */

    // Shadow map matrices.
    uniform mat4 headLMatrix;       // Left headlight light space matrix.
    uniform mat4 headRMatrix;       // Right headlight light space matrix.
    uniform mat4 rearLMatrix;       // Left rearlight light space matrix.
    uniform mat4 rearRMatrix;       // Right rearlight light space matrix.
    uniform mat4 reverseLMatrix;    // Left reverseLight light space matrix.
    uniform mat4 reverseRMatrix;    // Right reverseLight light space matrix.
    uniform mat4 highLMatrix;       // Left highBeam light space matrix.
    uniform mat4 highRMatrix;       // Right highBeam light space matrix.

    uniform mat4 CV_headLMatrix;       // Left headlight light space matrix  for the civilian car.
    uniform mat4 CV_headRMatrix;       // Right headlight light space matrix for the civilian car.
    uniform mat4 CV_rearLMatrix;       // Left rearlight light space matrix for the civilian car.
    uniform mat4 CV_rearRMatrix;       // Right rearlight light space matrix for the civilian car.

    // SHADOW_MAP_UNIT_BASE = 8
    uniform sampler2D headlightLShadowMap;      // Left headlight shadow map.
    uniform sampler2D headlightRShadowMap;      // Right headlight shadow map.
    uniform sampler2D rearlightLShadowMap;      // Left rear light shadow map.
    uniform sampler2D rearlightRShadowMap;      // Right rear light shadow map.
    uniform sampler2D reverseLightLShadowMap;   // Left reverse light shadow map.
    uniform sampler2D reverseLightRShadowMap;   // Right reverse light shadow map.
    uniform sampler2D highBeamLShadowMap;       // Left high beam shadow map.
    uniform sampler2D highBeamRShadowMap;       // Right high beam shadow map.

    uniform sampler2D CV_headlightLShadowMap;      // Left headlight shadow map for the civilian car.
    uniform sampler2D CV_headlightRShadowMap;      // Right headlight shadow map for the civilian car.
    uniform sampler2D CV_rearlightLShadowMap;      // Left rear light shadow map for the civilian car.
    uniform sampler2D CV_rearlightRShadowMap;      // Right rear light shadow map for the civilian car.

/* --- STOP SHADOW MAP VARIABLES --- */

/* --- START SHADOW CUBE VARIABLES --- */

    // SHADOW_CUBE_UNIT_BASE = 16
    uniform samplerCube lightBarLShadowCube;    // Left light bar shadow cube.
    uniform samplerCube lightBarRShadowCube;    // Right light bar shadow cube.
    uniform samplerCube blinkerFLShadowCube;    // Front left blinker shadow cube.
    uniform samplerCube blinkerFRShadowCube;    // Front right blinker shadow cube.
    uniform samplerCube blinkerBLShadowCube;    // Left light bar shadow cube.
    uniform samplerCube blinkerBRShadowCube;    // Right light bar shadow cube.
    uniform float lightBarFarPlane;             // The max distance the point shadows for the light bar.
    uniform float blinkerFarPlane;              // The max distance the point shadows for the blinker.

/* --- STOP SHADOW CUBE VARIABLES --- */

/* --- START CONSTANTS --- */

    const float ambientStrength = 0.1;  // Ambient strenght of light.
    const float specularStrength = 0.5; // Specular strenght of light.
    const float shininess = 32.0;       // Shininess of materials.

    // This isn't a evenly distrbuted sample pick, instead it's 'poisson-ish' (according to the forum I stole these from)
    const vec3 sampleDisk[8] = vec3[](
        vec3(0.5238, 0.3411, 0.4110),   vec3(-0.2411, 0.5620, -0.1566),
        vec3(-0.1566, -0.4828, 0.4021),  vec3(0.5419, -0.2041, -0.4411),
        vec3(0.1212, 0.1212, -0.9000),   vec3(-0.9000, 0.1212, 0.1212),
        vec3(0.1212, -0.9000, 0.1212),  vec3(0.0, 0.0, 0.0) // The center of the shadow looks sharp (0, 0, 0)
    );  // Sample disks for cube maps.

/* --- STOP CONSTANTS --- */

/*
    Interweaves gradient noise onto a fragment in order to make it look 'random', but not too grainy (as is the case for white noise).
    Apparently this is used by Activision?

    param: screenPos Screen position of the fragment

    return: The 'gradient noise' of the fragment
*/
float InterleavedGradientNoise(vec2 screenPos) 
{
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(screenPos, magic.xy)));
}

/*
    Rotates a 2D vector

    param: v Vector to rotate
    param: a Angle to rotate it by (degrees)

    return: The rotated vector
*/
vec2 rotate(vec2 v, float a) 
{
    float s = sin(a);
    float c = cos(a);
    return vec2(v.x * c - v.y * s, v.x * s + v.y * c);
}

/*
    Calculates the shadow for a shadow cube

    param: fragPos fragment position
    param: lightPos Position of the light
    param: depthCube The shadow cube depth
    param: farPlane The far plane

    return: The shadow value 
*/
float PointShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCube depthCube, float farPlane)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    
    // Spread more the farther it is
    float spread = (currentDepth / farPlane) * 0.15; 
    
    // Rotate
    float rotation = InterleavedGradientNoise(gl_FragCoord.xy) * 6.2831;
    float shadow = 0.0;
    
    float bias = 0.05; 

    for(int i = 0; i < 8; ++i)
    {
        // Rotate the sample disk points around the Y axis
        vec2 rotatedXZ = rotate(sampleDisk[i].xz, rotation);
        vec3 jitteredDir = vec3(rotatedXZ.x, sampleDisk[i].y, rotatedXZ.y);
        
        // Sample the cubemap with the jittered offset
        float closestDepth = texture(depthCube, fragToLight + jitteredDir * spread).r;
        closestDepth *= farPlane;
        
        if(currentDepth - bias > closestDepth)
        {
            shadow += 1.0;
        }
    }
    
    return shadow / 8.0;
}

/*
    Calculates the shadow for a shadow map

    param: shadowMap Texture of the shadow map
    param: fragPosLightSpace Fragment position in the light space of the shadow map
    param: NdotL Dot between normal and light direction
    param: normal Normal of the fragment

    return: The shadow value
*/
float ShadowCalculation(sampler2D shadowMap, vec4 fragPosLightSpace, float NdotL, vec3 normal)
{
    // Get the projection coordinates
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // If we are 'out' of the shadow map, no shadow
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }

    if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }

    // Apply a small offset to prevent moire (shadow acne)
    float offsetScale = 0.05;
    vec2 offset = normal.xz * (1.0 - NdotL) * offsetScale * (1.0 / textureSize(shadowMap, 0));
    
    // Calculate bias
    float bias = max(0.0003 * (1.0 - NdotL), 0.0003);

    // Get the depth of the projection
    float currentDepth = projCoords.z;

    // Get the texel size
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    float shadow = 0.0;

    // 9-piece PFC
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            // Check if the pixel counts for the shadow
            float pcfDepth = texture(shadowMap, projCoords.xy + offset + vec2(x, y) * (texelSize * 2.0)).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}

void main()
{
    // Normalize normal, get view direction and let's start this.
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);

    /* --- START MOONLIGHT --- */

    {
        // Normalize moon direction
        // (From surface to light)
        vec3 lightDir = normalize(-moonLight.direction);
        
        // Lighting Components
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        // Lighting itself
        vec3 ambient = ambientStrength * moonLight.color;
        vec3 diffuse = diff * moonLight.color * moonLight.intensity;
        vec3 specular = specularStrength * spec * moonLight.color;

        // Add 'em up
        result += (ambient + diffuse + specular);
    }

    /* --- STOP MOONLIGHT --- */

    /* --- START SPOTLIGHTS --- */

    // Matrices
    mat4 spotMatrices[12] = mat4[](
        headLMatrix, 
        headRMatrix, 
        rearLMatrix, 
        rearRMatrix, 
        reverseLMatrix, 
        reverseRMatrix, 
        highLMatrix, 
        highRMatrix,
        CV_headLMatrix, 
        CV_headRMatrix, 
        CV_rearLMatrix, 
        CV_rearRMatrix
    );

    for (int i = 0; i < spotLightCount; i++)
    {
        // Optimization
        if(spotLights[i].intensity <= 0.001f)
        {
            continue;
        }

        // Distance
        float dist = length(spotLights[i].position - FragPos);
    
        // Hard cutoff past 50.0 distance
        if (dist > 50.0)
        {
            continue; 
        }

        // Attenuation
        float attenuation = 1.0 / (spotLights[i].constant + spotLights[i].linear * dist + spotLights[i].quadratic * (dist * dist));
        
        // Optimization
        if (attenuation < 0.001)
        {
            continue;
        }

        // Light direction
        vec3 lightDir = normalize(spotLights[i].position - FragPos);

        // Spotlight cone check
        float theta = dot(lightDir, normalize(-spotLights[i].direction));
        if (theta < spotLights[i].outerCutOff)
        {
            continue; 
        }

        // Normal check
        float NdotL = max(dot(norm, lightDir), 0.0);
        if (NdotL <= 0.0) 
        {
            continue;
        }

        // Cone intensity, interpolate between inner and outer
        float epsilon = spotLights[i].cutOff - spotLights[i].outerCutOff;
        float spotIntensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);

        // Shadow mapping baby!
        float shadow = 0.0;
        int idx = spotLights[i].shadowMapIdx;

        if (idx >= 0 && idx < 12) 
        {
            // Matrix multiplication is done once using the array index
            vec4 lightSpacePos = spotMatrices[idx] * vec4(FragPos, 1.0);

            // Branching is only for the specific sampler unit
            if (idx == 0)      shadow = ShadowCalculation(headlightLShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 1) shadow = ShadowCalculation(headlightRShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 2) shadow = ShadowCalculation(rearlightLShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 3) shadow = ShadowCalculation(rearlightRShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 4) shadow = ShadowCalculation(reverseLightLShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 5) shadow = ShadowCalculation(reverseLightRShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 6) shadow = ShadowCalculation(highBeamLShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 7) shadow = ShadowCalculation(highBeamRShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 8) shadow = ShadowCalculation(CV_headlightLShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 9) shadow = ShadowCalculation(CV_headlightRShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 10) shadow = ShadowCalculation(CV_rearlightLShadowMap, lightSpacePos, NdotL, norm);
            else if (idx == 11) shadow = ShadowCalculation(CV_rearlightRShadowMap, lightSpacePos, NdotL, norm);
        }

        // Light components
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        vec3 amb = (ambientStrength * 0.1) * spotLights[i].color; 
        vec3 diffCol = NdotL * spotLights[i].color * spotLights[i].intensity;
        vec3 specCol = specularStrength * spec * spotLights[i].color * spotLights[i].intensity;

        // Apply shadow only to direct light, then scale by cone and attenuation
        // Shadow blocks everyhing but ambient
        result += (amb + (1.0 - shadow) * (diffCol + specCol)) * spotIntensity * attenuation;
    }

    /* --- STOP SPOTLIGHTS --- */

    /* --- STOP POINTLIGHTS --- */

    for (int i = 0; i < pointLightCount; i++)
    {
        // Optimizatione
        if(pointLights[i].intensity <= 0.001f)
        {
            continue;
        }

        // Distance
        float dist = length(pointLights[i].position - FragPos);
        if (dist > 30.0) 
        {
            continue;
        }
    
        // Get the far plane
        float currentFarPlane = (i < 2) ? lightBarFarPlane : blinkerFarPlane;
        // The distance over which it fades (1 unit)
        float window = 1.0;
        float farFade = clamp((currentFarPlane - dist) / window, 0.0, 1.0);

        // Attenuation
        float attenuation = (1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].quadratic * (dist * dist))) * farFade;
        if (attenuation < 0.001) 
        {
            continue;
        }

        // Light direction
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
    
        // Normal check
        float NdotL = max(dot(norm, lightDir), 0.0);
        if (NdotL <= 0.0)
        {
            continue;
        }

        // Point shadows baby!
        float shadow = 0.0;

        int idx = pointLights[i].shadowCubeIndex;

        if (idx >= 0 && idx < 6) 
        {
            if (idx == 0)      shadow = PointShadowCalculation(FragPos, pointLights[i].position, lightBarLShadowCube, lightBarFarPlane);
            else if (idx == 1) shadow = PointShadowCalculation(FragPos, pointLights[i].position, lightBarRShadowCube, lightBarFarPlane);
            else if (idx == 2) shadow = PointShadowCalculation(FragPos, pointLights[i].position, blinkerFLShadowCube, blinkerFarPlane);
            else if (idx == 3) shadow = PointShadowCalculation(FragPos, pointLights[i].position, blinkerFRShadowCube, blinkerFarPlane);
            else if (idx == 4) shadow = PointShadowCalculation(FragPos, pointLights[i].position, blinkerBLShadowCube, blinkerFarPlane);
            else if (idx == 5) shadow = PointShadowCalculation(FragPos, pointLights[i].position, blinkerBRShadowCube, blinkerFarPlane);
        }

        // Componenets
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        vec3 ambient = (ambientStrength * 0.05) * pointLights[i].color; 
        vec3 diffuse = NdotL * pointLights[i].color * pointLights[i].intensity;
        vec3 specular = specularStrength * spec * pointLights[i].color * pointLights[i].intensity;

        // Combine!
        result += (ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation;
    }

    // Get the color, of the texture if it has one, if not, then oh well
    vec4 sampledColor = (hasTexture == 1) ? texture(texture_diffuse, TexCoords) : vec4(PassColor, 1.0);
    
    // RGBA
    vec3 baseColor = sampledColor.rgb;
    float alpha = sampledColor.a;

    vec3 finalOut;

    // Check if full bright
    if (fullBright == 1)
    {
        finalOut = baseColor;
    }
    else
    {
        finalOut = result * baseColor;
    }

    // Check if light mesh
    if (isLightMesh == 1) 
    {
        finalOut += baseColor * lightMeshIntensity;
    }

    // Done!
    FragColor = vec4(finalOut, alpha);
}