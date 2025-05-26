# 计算机图形学入门概论

## 作业二：纹理和照明

**到期时间：** 2025年4月25日（星期五）晚上11:59

### I. 介绍

在本任务中，您需要使用OpenGL构建一个更加逼真和复杂的场景。您将体验OpenGL中的更多功能，包括照明、复杂模型构建和加载、纹理映射和交互式事件。您将使用基本体图形或直接从obj文件加载三维模型，然后应用查看/建模变换创建三维场景。通过纹理贴图和照明使场景和对象更加逼真。鼠标/键盘输入和窗口事件处理将帮助实现交互式动画。

**图1：** 演示中捕捉到的场景

在该指定场景中有两个模型：一个简单的背景雪原和一个复杂的企鹅模型。我们可以自己设计背景雪原的顶点属性。而对于企鹅，由于其复杂性，需要通过obj文件加载模型。雪原和企鹅将被渲染成不同的纹理和灯光效果，场景可通过用户交互控制。您可以进一步丰富在指定I中创建的场景。

### II. 实施详细信息

#### 任务1：加载复杂对象

使用Open Asset Import Library或我们提供的函数`Model loadOBJ(const char* objPath)`加载至少一个复杂模型（如演示程序中的企鹅）。您可以通过修改`void sendDataToOpenGL()`子例程来实现。

我们提供了模型文件`snowfield.obj`和`penguin.obj`。我们鼓励您从网络下载其他.obj文件或使用Blender设计您自己的对象。
（注意：使用penguin.obj时，需要进行适当的变换调整，因为默认尺寸非常大。）

#### 任务2：纹理映射和照明

将不同纹理映射到两个模型（雪地和企鹅）。我们将使用`stb_image`（见"依赖项/stb_image"）来加载纹理图像。您需要实现通过键盘交互更改企鹅纹理的功能。

首先，生成OpenGL纹理并通过修改`void texture::setupTexture(const char* texturePath)`子例程设置纹理参数。然后，在`void sendDataToOpenGL()`和`void paintGL(void)`子例程中加载纹理并将其绑定到不同模型。

我们提供了两个模型的纹理，也鼓励您从网络下载其他纹理或自己创建。

此外，3D场景应使用至少两个光源：一个环境（定向）光和一个您自定义的光源（位置和颜色自选）。目的是在模型上产生漫反射和镜面反射效果。修改`void paintGL(void)`子例程来完成此任务。

#### 任务3：互动活动和动画

实现以下交互式事件和动画：

**(a) 照明控制**
- 按`w`键和`s`键分别增加和减少定向光的亮度

**(b) 纹理控制**
- 按`1`和`2`键为企鹅切换两种不同纹理（`penguin/dolphin_01.jpg`，`penguin/penguin_02.jpg`）
- 按`3`和`4`键为雪原切换两种不同纹理（`snowfield/snowfield_01.jpg`，`snowfield/snowfield_02.jpg`）

**(c) 对象控制**
- 使用箭头键`↑↓←→`控制企鹅移动：`↑↓`分别表示前进和后退；`←→`分别表示向左和向右旋转

**(d) 视图控制**
- 通过鼠标控制摄像机视图：按住左键时，鼠标上下移动使场景相应地上下移动

修改以下子程序实现上述功能：

```cpp
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Sets the mouse-button callback for the current window.
}

void cursor_position_recallback(GLFWwindow* window, double x, double y)
{
    // Sets the cursor position callback for the current window
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Sets the scroll callback for the current window.
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Sets the Keyboard callback for the current window.
}
```

#### 额外任务：增强场景的视觉效果（最高20%）

OpenGL提供了许多创建各种视觉效果的功能。以下是一些建议改进：
- 加载更复杂的模型并应用其他纹理，形成有意义的场景（10%）
- 使用不同类型光源（如点光源、聚光灯等）组合创建有意义的场景（10%）
- 在复杂模型上实现阴影映射（10%）
- 绘制点或线跟踪复杂模型的运动轨迹（10%）
- 其他有趣效果（10%）

### III. 评分方案

您的作业将按照以下标准评分：

**基本部分（80%）**
- 加载复杂模型：10%
- 纹理贴图：10%
- 环境（定向）光照明：10%
- 自定义光源照明：10%
- 照明控制：10%
- 纹理控制：10%
- 对象控制：10%
- 视图控制：10%

**额外功能：** 20%

**总计：** 100%

**注意：** 程序不完整或编译失败将不予评分。

### IV. 提交指南

1) 官方评分平台为Windows with Visual Studio。如有执行/编译问题，助教会联系您重新提交。

2) 修改提供的`main.cpp`、`VertexShaderCode.glsl`和`FragmentShaderCode.glsl`。如需实现阴影映射，可创建额外.glsl文件。也鼓励为相机类添加额外的.h和.cpp文件。在`main.cpp`中填写您的全名和学号，缺少这些信息将扣分（最高10分）。

3) 仅接受可编程管道中编写的OpenGL代码，固定管道实现不予评分。

4) 仅接受使用GLFW和GLEW实现的OpenGL代码，其他窗口和OpenGL扩展库实现不予评分（除非有充分理由）。

5) 将源代码文件、可执行文件、自述文件（readme.txt）、.obj文件和图像文件压缩到.zip文件中，以您的学号命名（例如：20210810XXX+张三.zip）。

6) 请在截止日期前（晚上11:59）通过电子邮件提交作业给助教。

7) 多份提交只考虑最新一份。

8) 抄袭将导致课程不及格。
