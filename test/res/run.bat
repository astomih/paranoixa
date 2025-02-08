dxc -T ps_6_6 -E main -Fo shader.frag.cso shader.frag.hlsl
dxc -T vs_6_6 -E main -Fo shader.vert.cso shader.vert.hlsl
python compileShader.py
