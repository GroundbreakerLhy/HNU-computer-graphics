以下是将您提供的PDF内容转换为Markdown格式：

# 计算机图形学入门

## 课程项目:太空黄金收藏

**截止时间**: 2025年5月24日晚上11:59

不允许逾期提交

如果您抄袭则不通过课程

## 1. 介绍

在5202年,天文学家发现了一颗周围有陨石的行星,那里有一些珍贵的黄金。作为一名杰出的飞行员,你正在驾驶一艘宇宙飞船获得这些金牌。然而,当地人的几辆太空车正在四处走动并保护他们。你需要避开他们并收集金牌。

图1:演示程序绘制的场景。

为了完成这个项目,您需要从头开始编写自己的代码。您需要的所有基本技术已经或将在我们的教程中介绍。您最好的框架代码是作业1和作业2的解决方案程序。

这个项目的最终目标是让你有机会更多地练习计算机图形学中的基本但非常重要的主题:你必须经历对象加载、变换矩阵、照明、纹理映射、skybox、着色器和交互,然后才能获得满意的分数。

---

## II. 实施详细信息

### 1. 基本要求:

a) 用相应的纹理渲染行星、航天器和至少三个本地太空飞行器。为了简化,请将它们的质心保持在与世界空间的一个轴垂直的平面上。
b) 地球和当地的太空飞行器应该一直自转。
c) 本地空间飞行器应该一直在水平轴上移动。
d) 创建一个skybox作为虚拟场景的背景。
e) 创建点光源。应在所有对象上明显观察到基本的灯光渲染(环境光、漫反射和镜面反射)。请正确设置照明参数,以便进行清晰的演示。在演示过程中,您可以通过键盘交互来调整灯光参数。
f) 生成一个包含至少200块随机漂浮岩石的小行星环云。这些漂浮的岩石应该在有限的范围内有随机的位置。
g) 视点应在航天器尾部后方,并与其相对固定。观察方向应与航天器头部指向的方向一致。观看我们的演示视频以获得直观的说明。
h) 对于交互:
   1) 鼠标:使用鼠标控制航天器的旋转。例如,如果向左移动鼠标,航天器的头部将向左转动。
   2) 键盘:请使用以下四个键来控制航天器的平移:
      i. 向上光标键:将航天器向前移动一定距离。
      ii. 向下光标键:将航天器向后移动一定距离。
      iii. 向左光标键:将航天器向左移动一定距离。
      iv. 向右光标键:将航天器向右移动一定距离。
   3) 与环境互动:
      i. 避开本地太空飞行器。当地的太空飞行器正在四处移动并搜索你。如果他们抓到你,它会被提醒(纹理变化)。(提示:碰撞检测)。
      ii. 收集金牌。寻找隐藏在地球周围岩石中的金子。通过靠近它们来收集它们。它们被收集后会消失或腐烂成岩石。
      iii. 完成收集。在你完成所有收集任务后,你的航天器的纹理应该改为金色。

图2:向本地太空飞行器发出警报之前和之后。

### 2. 额外要求:

a) 添加另一个光源。两个光源的基本光渲染结果应根据Phong Illumination Model的求和特性确定。
b) 为行星做法线贴图。我们为这个星球提供了一张正常的地图。应该加载行星纹理图像和法线贴图图像,以便在片段着色器中使用它们。
c) 丰富宝藏。我们只将黄金作为收藏品。您可以在线下载其他obj和纹理文件,以丰富宝藏的类型。

图3:完成所有收集任务后的金色太空船和周围有岩石的行星。

## III. 框架和文件

1. 我们提供了该项目中物体(行星、航天器、岩石、太空飞行器)的基本.obj文件。还提供了相应的纹理图像。
2. 我们提供一个演示视频。一定要仔细观看,以充分理解我们的要求。
3. 作业1和作业2的解决方案程序应该为您提供一个良好的起点。这个项目中的大多数任务都可以分解为简单的任务,这些任务已经在我们的讲座和教程中教授过了。我们为您提供一些建议:
   * 掌握模型坐标系、世界坐标系和相机坐标系之间的转换知识。
   * 掌握渲染管道、VAO和VBO的良好知识。你可能会因为同时处理这么多对象而感到困惑。尝试使用VAO和VBO来帮助您解决问题,因为渲染对象的各种信息可以附加到这些项中。
   * 尽量保持整洁的编码风格。干净的编码风格有助于调试。通过适当的注释保持头脑清醒。此外,请尝试将重复的代码封装到函数中。
4. 推荐的库:
   * 用于创建和管理包含OpenGL上下文的窗口的FreeGLUT。
   * GLEW,用于查询和加载OpenGL扩展。
   * GLM是一个用于图形软件的C++数学库。
   * 协助从.obj文件加载三维对象模型。
   * 用于从图像加载纹理的土壤。

---

## IV. 汇报

准备一个.pdf文件,包括以下部分:
1. 如图1所示,该图显示了整个场景。
2. 提供对每种对象的基本灯光渲染结果的近距离观察的帧。
3. 显示航天器(1)向当地车辆发出警报,(2)收集金牌,(3)收集后改变航天器纹理的框架。
4. 可以表示您已实现的任何奖励功能的框架。
5. 对您的实现细节进行一些简短而必要的描述。

## V. 分级方案

您的作业将按照以下评分方案进行评分:

| 项目                 | 描述                                     | 分数   |
| :------------------- | :--------------------------------------- | :----- |
| **基本(88%)**        |                                          |        |
| 1                    | 渲染一颗行星、一个航天器和至少三个航天器 | 10%    |
| 2                    | 行星和本地空间飞行器的自转             | 6%     |
| 3                    | 渲染天空框                               | 6%     |
| 4                    | 基本灯光渲染                             | 4%     |
| 5                    | 渲染小行星环形云                         | 10%    |
| 6                    | 岩石的旋转                               | 8%     |
| 7                    | 正确的观点                               | 8%     |
| 8                    | 使用鼠标控制航天器的自转                 | 8%     |
| 9                    | 使用键盘控制航天器的平移                 | 8%     |
| 10                   | 收集金牌                                 | 8%     |
| 11                   | 警告后更改本地车辆的纹理                 | 8%     |
| 12                   | 完成整个收集后更改航天器的纹理           | 4%     |
| **额外要求(12%)**    |                                          |        |
| 1                    | 添加另一个光源                           | 5%     |
| 2                    | 行星的法线贴图                           | 5%     |
| 3                    | 更多种类的宝藏来代替提供的黄金。         | 2%     |
| **总计:**            |                                          | **100%** |

**注**: 如果在演示过程中程序不完整或未能编译,将给予相当大的分数扣减。

## VI. 项目提交和演示指南

1. 自行组队,每组1-2个同学,5.4号之前和助教汇报组队人员学号,过期视为个人独立完成。
2. 将项目文件(.h、.cpp、shaders、new obj/bmp(如果有))和项目报告压缩到.zip文件中。用你的学号(例如2020XXXXX_2020XXXXX.zip)命名,并在5月25日23:59之前提交到助教邮箱。一个小组中只需要提交一份,备注好小组成员。不允许延迟提交。
3. 项目演示将在2025年5月28日和6月4日这两次课堂时间进行,每组5分钟左右。
4. 在演示过程中会提出一些问题。您将被要求解释程序中的一些代码,并讨论这些功能是如何实现的。