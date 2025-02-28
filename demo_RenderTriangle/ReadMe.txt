We provide three files for win and mac, separately.

How to use the provided three files for Win:
	1. Create a new Visual Studio project folder and set up the OpenGL environment.
	2. Copy three files into your Visual Studio project folder, and then add the three files under the "Source File" folder (Right click the "Source File" folder, then choose Add -> Existing item, add the three files).
	3. Open "VertexShaderCode.glsl". In the first line, "#version 430", replace "430" with the GLSL version that your computer supports. Also for the "VertexShaderCode.glsl".
	4. Compile and Run.

It's similar for Mac expect for:
	1. Xcode project.
	2. Add files from the Xcode project panel.
	3. Do NOT modify "version 330 core" unless you know what you are doing.


How to control the triangle:
	Press "a" to move left; 
	Press "d" to move right.
