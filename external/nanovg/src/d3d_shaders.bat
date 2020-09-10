rem Compiles D3D shaders to headers - only needed if you change the hlsl.
rem The headers are checked in....

fxc.exe /Fh D3D11PixelShader.h /T ps_4_0_level_9_3 /E D3D11PixelShader_Main D3D11PixelShader.hlsl
fxc.exe /Fh D3D11PixelShaderAA.h /T ps_4_0_level_9_3 /E D3D11PixelShaderAA_Main D3D11PixelShaderAA.hlsl
fxc.exe /Fh D3D11VertexShader.h /T vs_4_0_level_9_3 /E D3D11VertexShader_Main D3D11VertexShader.hlsl
