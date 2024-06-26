# Screen Space Directional Occlusion

还是经典的 Global Illumination = Direct Illumination + Indirect Illumination 思路，

可以先简单地分为若干部分：

1. Direct Illumination: 直接光
2. Ambient Occlusion: 环境光遮蔽
3. Directional Occlusion: 

## DI

当光源情况简单时，一般可以有简单的方式处理（如 Shadow mapping）。

当光源复杂时，采样近似：

$$
L_{dir}(P)= \sum_{i=1}^N \text{BRDF}(\omega_i) L_{in}(\omega_i) V(\omega_i) \cos \theta_i \Delta \omega_i
$$

假设我们直接从光源上采样 $\omega_i$，那么 $L_{in}(P,\omega_i)$ 就是从光源信息上提取。

现在只需要求 Visibility 项，为了避免在全局做光追，论文里用 Screen Space Approximation 的方法。

1. 随机一个 step $\lambda_i \in [0,r_{max}]$.
2. 通过 depth 检查 $P+\lambda_i \omega_i$ 是否在表面以下，如果在就认为是 Occluder $V(\omega_i)=0$.

> 注：论文里直接用 $\rho/\pi$ 代表 Diffuse 材质的 BRDF.

## One-bounce Indirect Light

在上一步中，没有被遮挡的位置可以计算直接光；被遮挡的位置，根据采样到的 Occluder 可以映射到一个 pixel，可以考虑从这个 pixel 计算 one-bounce light。

就是将这个 Pixel 对应的 fragment 看成是 Diffuse 材质，从而当做极小的面光源处理。

$$
L_{ind}(P)= \sum_{i=1}^N \text{BRDF}(i') L_{pixel}(\omega_i) (1-V(\omega_i)) \frac{A_s \cos \theta_{s_i} \cos \theta_{r_i}}{d_i^2}
$$

记 Occluder （即采样点在 depth map 上映射到的）的 World Position 为 $Q$，$i'$ 表示 $Q$ 到 $P$ 的光线。

1. 这里 $d_i$ 是 $P$ 和 $Q$ 的距离，一般和 $1$ 取 max 防止出现 bug。
2. $\theta_{s_i}$ 和 $\theta_{r_i}$ 分别是 sender(pixel 的 normal)、Receiver (P 点的 normal) 和 $i'$ 的夹角。
3. $A_s$ 就是 sender (pixel) 的面积。

$A_s$ 可以取 $\pi r_{max}^2/N$，也可以手动调整来调整 Color Bleeding 的强度。

（没太搞懂，如果手动调的话感觉多次迭代会出问题，严重偏离 Radiometry 量纲）

## 优化

1. Depth Peeling：存储多个 depth 值。

2. Multiple View：在多个方向渲染第一个 Pass，以保留更多的 "Screen Space" 信息。

3. Shadow Mapping Bias: $b=p/2 \cdot \tan(\alpha)$，$p$ 表示 pixel 在 world space 的长度，$\alpha$ 表示光线和法线的夹角. （用 Depth Peeling 会更准确就是了）。





