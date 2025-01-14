struct VertexOutput {
    @location(0) outColor: vec3<f32>,
    @location(1) outUV: vec2<f32>,
    @builtin(position) gl_Position: vec4<f32>,
}

var<private> inPosition_1: vec3<f32>;
var<private> inUV_1: vec2<f32>;
var<private> inColor_1: vec3<f32>;
var<private> outColor: vec3<f32>;
var<private> outUV: vec2<f32>;
var<private> gl_Position: vec4<f32>;

fn main_1() {
    let _e6 = inPosition_1;
    gl_Position = vec4<f32>(_e6.x, _e6.y, _e6.z, 1f);
    let _e13 = inColor_1;
    outColor = _e13;
    let _e14 = inUV_1;
    outUV = _e14;
    return;
}

@vertex 
fn main(@location(0) inPosition: vec3<f32>, @location(1) inUV: vec2<f32>, @location(2) inColor: vec3<f32>) -> VertexOutput {
    inPosition_1 = inPosition;
    inUV_1 = inUV;
    inColor_1 = inColor;
    main_1();
    let _e17 = outColor;
    let _e19 = outUV;
    let _e21 = gl_Position;
    return VertexOutput(_e17, _e19, _e21);
}
