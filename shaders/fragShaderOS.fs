#version 430 core
const float PI = 3.1415926535897932384626433832795f;

layout (binding=0) uniform sampler2D currentTextureFlow;

in vec2 TexCoord;
in vec2 meanFlow;

layout (location = 0) out vec4 color;

uniform int texLevel;




void main()
{
	vec2 texSize = vec2(textureSize(currentTextureFlow, texLevel).xy);

	//float u = (2.0 * float(gl_FragCoord.x)) / texSize.x - 1.0f; //1024.0f is the window resolution, change this to a uniform
    //float v = (2.0 * float(gl_FragCoord.y)) / texSize.y - 1.0f;

	float u = float(gl_FragCoord.x); // 0 - windowSize (1920)
    float v = float(gl_FragCoord.y); // 0 - 1080

	vec2 tFlow = textureLod(currentTextureFlow, vec2((u / 1920.0f), 1.0 - (v / 1080.0f)), 0).xy - meanFlow;

	//	color = vec4(1.0f, 0.9, 0.8, 1.0);

	color = vec4(meanFlow.x * meanFlow.x, meanFlow.y * meanFlow.y, 0, 1);

	//color = vec4(tFlow.x * tFlow.x, tFlow.y * tFlow.y, 0, 1); 

}
