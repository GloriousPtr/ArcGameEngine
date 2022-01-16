#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

uniform sampler2D u_Texture;
uniform sampler2D u_DepthTexture;

uniform mat4 u_InverseProjMatrix;
uniform int u_Samples;
uniform float u_IndirectAmount;
uniform float u_NoiseAmount;
uniform bool u_Noise;

layout(location = 0) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_FragColor;

vec2 mod_dither3(vec2 u)
{
	float u_NoiseX = mod(u.x + u.y + mod(208. + u.x * 3.58, 13. + mod(u.y * 22.9, 9.)),7.) * .143;
	float u_NoiseY = mod(u.y + u.x + mod(203. + u.y * 3.18, 12. + mod(u.x * 27.4, 8.)),6.) * .139;
	return vec2(u_NoiseX, u_NoiseY)*2.0-1.0;
}

vec2 dither(vec2 coord, float seed, vec2 size)
{
	float u_NoiseX = ((fract(1.0-(coord.x+seed*1.0)*(size.x/2.0))*0.25)+(fract((coord.y+seed*2.0)*(size.y/2.0))*0.75))*2.0-1.0;
	float u_NoiseY = ((fract(1.0-(coord.x+seed*3.0)*(size.x/2.0))*0.75)+(fract((coord.y+seed*4.0)*(size.y/2.0))*0.25))*2.0-1.0;
    return vec2(u_NoiseX, u_NoiseY);
}


vec3 getViewPos(sampler2D tex, vec2 coord, mat4 ipm)
{
	float depth = texture(tex, coord).r;
	
	//Turn the current pixel from ndc to world coordinates
	vec3 pixel_pos_ndc = vec3(coord*2.0-1.0, depth*2.0-1.0); 
    vec4 pixel_pos_clip = ipm * vec4(pixel_pos_ndc,1.0);
    vec3 pixel_pos_cam = pixel_pos_clip.xyz / pixel_pos_clip.w;
	return pixel_pos_cam;
}


vec3 getViewNormal(sampler2D tex, vec2 coord, mat4 ipm)
{
    ivec2 texSize = textureSize(tex, 0);

    float pW = 1.0/float(texSize.x);
    float pH = 1.0/float(texSize.y);
    
    vec3 p1 = getViewPos(tex, coord+vec2(pW,0.0), ipm).xyz;
    vec3 p2 = getViewPos(tex, coord+vec2(0.0,pH), ipm).xyz;
    vec3 p3 = getViewPos(tex, coord+vec2(-pW,0.0), ipm).xyz;
    vec3 p4 = getViewPos(tex, coord+vec2(0.0,-pH), ipm).xyz;

    vec3 vP = getViewPos(tex, coord, ipm);
    
    vec3 dx = vP-p1;
    vec3 dy = p2-vP;
    vec3 dx2 = p3-vP;
    vec3 dy2 = vP-p4;
    
    if(length(dx2)<length(dx)&&coord.x-pW>=0.0||coord.x+pW>1.0)
	{
		dx = dx2;
    }
    if(length(dy2)<length(dy)&&coord.y-pH>=0.0||coord.y+pH>1.0)
	{
		dy = dy2;
    }
    
    return normalize(-cross( dx , dy ).xyz);
}

float lenSq(vec3 vector)
{
    return pow(vector.x, 2.0) + pow(vector.y, 2.0) + pow(vector.z, 2.0);
}

vec3 lightSample(sampler2D color_tex, sampler2D depth_tex,  vec2 coord, mat4 ipm, vec2 lightcoord, vec3 normal, vec3 position, float n, vec2 texsize)
{
	vec2 random = vec2(1.0);
	if (u_Noise)
	{
    	random = (mod_dither3((coord*texsize)+vec2(n*82.294,n*127.721)))*0.01*u_NoiseAmount;
	}
	else
	{
		random = dither(coord, 1.0, texsize)*0.1*u_NoiseAmount;
	}
    lightcoord *= vec2(0.7);
    
    //light absolute data
    vec3 lightcolor = textureLod(color_tex, ((lightcoord)+random),4.0).rgb;
    vec3 lightnormal   = getViewNormal(depth_tex, fract(lightcoord)+random, ipm).rgb;
    vec3 lightposition = getViewPos(depth_tex, fract(lightcoord)+random, ipm).xyz;

    
    //light variable data
    vec3 lightpath = lightposition - position;
    vec3 lightdir  = normalize(lightpath);
    
    //falloff calculations
    float cosemit  = clamp(dot(lightdir, -lightnormal), 0.0, 1.0); //emit only in one direction
    float coscatch = clamp(dot(lightdir, normal)*0.5+0.5,  0.0, 1.0); //recieve light from one direction
    float distfall = pow(lenSq(lightpath), 0.1) + 1.0;        //fall off with distance
    
    return (lightcolor * cosemit * coscatch / distfall)*(length(lightposition)/20.0);
}

void main()
{
	vec3 direct = textureLod(u_Texture, v_TexCoord, 0.0).rgb;
    vec3 color = normalize(direct).rgb;
    vec3 indirect = vec3(0.0, 0.0, 0.0);
    float PI = 3.14159;
    ivec2 iTexSize = textureSize(u_Texture, 0);
    vec2 texSize = vec2(float(iTexSize.x),float(iTexSize.y));
    //fragment geometry data
    vec3 position = getViewPos(u_DepthTexture, v_TexCoord, u_InverseProjMatrix);
    vec3 normal   = getViewNormal(u_DepthTexture, v_TexCoord, u_InverseProjMatrix);
    
    //sampling in spiral
    
    float dlong = PI*(3.0-sqrt(5.0));
    float dz = 1.0/float(u_Samples);
    float long = 0.0;
    float z = 1.0 - dz/2.0;
    
    
    for(int i = 0; i < u_Samples; i++){
            
        float r = sqrt(1.0-z);
        
        float xpoint = (cos(long)*r)*0.5+0.5;
        float ypoint = (sin(long)*r)*0.5+0.5;
                
        z = z - dz;
        long = long + dlong;
    
        indirect += lightSample(SCREEN_TEXTURE, DEPTH_TEXTURE, SCREEN_UV, INV_PROJECTION_MATRIX, vec2(xpoint, ypoint), normal, position, float(i), texSize); 

        }

	o_FragColor = vec4(direct+(indirect/float(u_Samples) * u_IndirectAmount), 1.0);
}
