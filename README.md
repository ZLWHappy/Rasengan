# Rasengan
A simple visualization implementaion of the rasengan of Naruto

there is no complict techniques:

    1.If you are familiar with OpenSceneGraph, I believe you can just download the source code and run it in a new program

    2.I use the instancing model to draw lots of geometries

    3.In the geometry shader, I change the input triangle into long thin strip

    4.Some random effect was created using time function and trigonometric function


这是一个利用OpenSceneGraph开源图形引擎编写的简单的《火影忍者》中漩涡鸣人的绝招螺旋丸的可视化例子。

这个例子并没有使用很复杂的技术：

    1.如果你用过OpenSceneGraph，你可以下载源码并自己新建项目运行该例子

    2.用到了OpenSceneGraph/OpenGL的实例化来渲染

    3.在几何着色器阶段改变了输入顶点着色器图元的形状：由小三角形变成了细长的条带

    4.利用时间函数和三角函数生成了一些随机的效果

