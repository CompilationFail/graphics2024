# 计算机图形学 SSDO

- 项目简介  

  本项目是利用**屏幕空间定向遮蔽**（Screen-Space Directional Occlusion，SSDO）技术实现的图形渲染程序.  

  本项目在实现了SSDO的同时，允许用户自行设置某些技术参数，并可以选择不同的模型、以任意视角观察渲染结果.

* 已完成部分
  1. 从头到尾的渲染管线。
  2. obj 格式的模型加载和对应的材质加载， scene 格式的场景加载。
  3. 点光源，平行光，cone 状光源的多光源（< 10 个）渲染。
  4. PBR 材质。
  5. PCF soft shadow.
  6. SSDO 近似光照（仅计算 1 bounce）。
  7. 对于间接光使用了 denoise。

- 第三方库：

  1. glew / glfw / glm: 基础库
  2. imgui: UI 库
  3. stb: 用于加载 texture

- 编译和执行

  本项目可以使用cmake进行编译.在源代码文件夹下执行如下命令即可编译代码.

   ```bash
    mkdir build 
    cd build
    cmake ..
    cd ..
    cmake --build build
   ```

  向生成的可执行文件输入scene所在的路径即可，根目录下提供了 `1.scene` 和 `2.scene`.

  运行示例:

  ```bash
  main.exe 1.scene
  ```

- 交互方式

1. 视点和视角的自由变换

​	在英文输入法下按下键盘上的 `W/S/A/D` 键，可以实现摄像机的 前/后/左/右 移动.

2. 调整属性

​	M键可以呼出鼠标，然后按U呼出UI菜单，在UI菜单调整属性。

* 测试环境

  1. 分辨率：1920 x 1080px.
  2. 测试机器：Win11, i5-13600KF, RTX4080


* 存在的问题

  1. SSDO 的采样策略较差（此处为效率瓶颈）。
  2. 仅 Pooling 降噪效果依然不佳，加入 Temporal 后在移动时无法保持高质量的间接光效果。
  3. 因为没有实现SSR，所以倒影质量较低

* 分工

  基于之前小作业的代码，两人共同完成 SSDO 部分
* 效果展示
  ![demo](/models/demo.jpg)
