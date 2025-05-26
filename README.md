# 湖南大学计算机图形学2025春季作业

## Install

**Linux (Ubuntu/Debian)**
```bash
git clone https://github.com/GroundbreakerLhy/HNU-computer-graphics.git
cd HNU-computer-graphics
git submodule update --init --recursive
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential cmake
sudo apt-get install libglfw3-dev libglew-dev libglm-dev
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
```

**交叉编译到 Windows (Linux 环境下)**
```bash
git clone https://github.com/GroundbreakerLhy/HNU-computer-graphics.git
cd HNU-computer-graphics
git submodule update --init --recursive
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential cmake
sudo apt-get install mingw-w64 mingw-w64-tools
# Windows版本的库文件已包含在 Dependencies 目录中
```

## Usage
**Linux**
```bash
cd <project_directory>
make
./<executable_name>
```
**交叉编译 Windows 可执行文件**
```bash
cd <project_directory>/for\ win 
make -f makefile.win
./<executable_name>.exe
```
## Directory Structure
```
|-- Assignment1
    |-- skeleton code
        |-- for win
        |-- ...
    |-- ...
|-- Assignment2
    |-- skeleton code
        |-- for win
        |-- ...
    |-- ...
|-- CourseProject
    |-- for win
    |-- ...
|-- demo_RenderTriangle
    |-- ...
|-- Dependencies
    |-- ...
|-- imgui
    |-- ...
|-- README.md
```