# graphics2024

Source code for graphics 2024 course homework.

使用的第三方库：

1. glew / glfw / glm: 基础库
2. imgui: UI 库
3. stb: 用于加载 texture

已完成的部分：

1. 从头到尾的渲染管线。
2. obj 格式的模型加载和对应的材质加载， scene 格式的场景加载。
3. 点光源，平行光，cone 状光源的多光源（< 10 个）渲染。
4. PBR 材质。
5. PCF soft shadow.
6. SSDO 近似光照（仅计算 1 bounce）。
7. 对于间接光的 Spatial pooling denoising.

测试环境为：

1. 分辨率：1920 x 1080px.
2. 测试机：Win11, i5-13600KF, RTX4080

存在的问题：

1. SSDO 的采样策略较差，导致采样数较高（此处为效率瓶颈）。
2. 降噪效果依然不佳，存在噪点闪烁和条纹。
3. Gamma 矫正后丢失精度导致地面光照出现圈状。
