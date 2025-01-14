struct FragmentOutput {
    @location(0) outColor: vec4<f32>,
}

var<private> inColor_1: vec3<f32>;
var<private> inUV_1: vec2<f32>;
var<private> outColor: vec4<f32>;
@group(0) @binding(0) 
var tex: texture_2d<f32>;
@group(0) @binding(1) 
var samp: sampler;

fn main_1() {
    var f: vec2<f32>;

    let _e5 = inUV_1;
    let _e8 = inUV_1;
    f = vec2<f32>(_e5.x, (1f - _e8.y));
    let _e13 = inUV_1;
    f = _e13;
    let _e15 = f;
    let _e16 = textureSample(tex, samp, _e15);
    outColor = _e16;
    return;
}

@fragment 
fn main(@location(0) inColor: vec3<f32>, @location(1) inUV: vec2<f32>) -> FragmentOutput {
    inColor_1 = inColor;
    inUV_1 = inUV;
    main_1();
    let _e15 = outColor;
    return FragmentOutput(_e15);
}
