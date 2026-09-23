struct VSOutput
{
    float4 posCS : SV_POSITION;
    float2 uv : TEXCOORDS;
};

Texture2D g_entireScreenImage : register(t0);
// sampler g_pointSampler : register(s0);

Texture2D g_uiMaskSRV : register(t1);
//Texture2D g_bunnySRV : register(t2);

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
    
    // UI areas dont touch! (mask texture)

    // Remap color.rgb into new range (same as Adjustments > Levels in Paint.net),
    // so we will remap color.rgb into [minThreshold, maxThreshold] range:
    // pixel with minLuminanceThreshold color will be new 0.0f, and pixel with color
    // maxThreshold will be new 1.0f:
    float maxThreshold = 0.25f;
    float minThreshold = 0.014f;
    
    float3 invRange = 1.0f / max(maxThreshold - minThreshold, 0.0001f);
    float3 remapped = (color.rgb - minThreshold) * invRange;
    remapped = clamp(remapped, 0.0f, 1.0f);
    
    // Apply gamma correction:
    float gamma = 4.0f;
    float3 gammaCorrected = pow(remapped, 1.0f / max(gamma, 0.0001f));
    
    // Convert to grayscale:
    float luminance = dot(gammaCorrected.rgb, float3(0.299f, 0.587f, 0.114f));
    
    // Avoid applying gamma to UI:
    float uiMask = g_uiMaskSRV.Load(int3(x, y, 0));
    float3 masked = lerp(color.rgb, luminance.rrr, uiMask);
    
    float alpha = 1.0f;
   
    // Just for fun:
    //Rect bunnyPos = { 1500, 985, 60, 100 };
    
    //int bunnyLocalX = x - bunnyPos.left;
    //int bunnyLocalY = y - bunnyPos.top;
    
    //if (bunnyLocalX >= 0 && bunnyLocalX < bunnyPos.width &&
    //    bunnyLocalY >= 0 && bunnyLocalY < bunnyPos.height)
    //{
    //    float4 bunny = g_bunnySRV.Load(int3(bunnyLocalX, bunnyLocalY, 0));
    //    
    //    if (bunny.a > 0.0f)
    //    {
    //        masked = bunny.rgb;
    //        alpha = bunny.a;
    //    }
    //}
    
    return float4(masked.rgb, alpha);
}
