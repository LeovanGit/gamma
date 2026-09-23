struct VSOutput
{
    float4 posCS : SV_POSITION;
    float2 uv : TEXCOORDS;
};

Texture2D g_entireScreenImage : register(t0);
// sampler g_pointSampler : register(s0);

struct Rect
{
    uint left;
    uint top;
    uint width;
    uint height;
};

VSOutput FullscreenTriangleVS(uint vertexIndex : SV_VertexID)
{
    float2 fullscreenTrianglePos[] =
    {
        { -1.0f, -1.0f },
        { -1.0f,  3.0f },
        {  3.0f, -1.0f }
    };
    
    float2 fullscreenTriangleUV[] =
    {
        { 0.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f }
    };

    VSOutput output;
    output.posCS = float4(fullscreenTrianglePos[vertexIndex].xy, 0.0f, 1.0f);
    output.uv = fullscreenTriangleUV[vertexIndex].xy;

    return output;
}

float4 GammaPS(VSOutput input) : SV_TARGET0
{
    // For performance we will copy and bind entire screen image (using CopyResource()),
    // so we need to cut it to our window (render target) sizes here:
    Rect g_windowSize = { 0, 0, 1920, 1080 };
    
    int x = input.posCS.x + g_windowSize.left;
    int y = input.posCS.y + g_windowSize.top;
    
    float4 color = g_entireScreenImage.Load(int3(x, y, 0));
    
    float result = 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
    
    return float4(result.rrr, 1.0f);
}
