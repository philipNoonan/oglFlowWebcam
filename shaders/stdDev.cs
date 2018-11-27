#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0, rg32f) uniform image2D prefixSumFlow;

layout(std430, binding = 0) buffer quadlistBuf
{
    vec4 quadlist [];
};

layout(std430, binding = 1) buffer quadlistMeanTempBuf
{
    vec2 outputMeanTemp [];
};

void main()
{
    uint index = gl_GlobalInvocationID.x;

    ivec2 pos = ivec2(quadlist[index].x, quadlist[index].y);

    //uint xPos = uint(quadlist.x);
    //uint yPos = uint(quadlist.y);
    uint lod = uint(quadlist[index].z);

    float quadSideLength = float(pow(2, lod)); //

    //vec2 origin = ((vec2(xPos, yPos) * quadSideLength) + (quadSideLength * 0.5f)); // 

    float A, B, C, D;

    // for a quad of the prefix sum image where
    //      A -- B
    //      -    -
    //      -    -
    //      C -- D
    // sum of quad of the original image = A + D - B - C

    vec2 xSum = imageLoad(prefixSumFlow, pos).xy + 
                imageLoad(prefixSumFlow, ivec2(pos.x + quadSideLength, pos.y + quadSideLength)).xy -
                imageLoad(prefixSumFlow, ivec2(pos.x + quadSideLength, pos.y)).xy - 
                imageLoad(prefixSumFlow, ivec2(pos.x, pos.y + quadSideLength)).xy;

    vec2 xMean = xSum / (quadSideLength * quadSideLength);

    outputMeanTemp[index] = xMean;




}
