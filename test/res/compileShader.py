# Get file lists and compile shaders

import os
import subprocess
import sys

# Get the current working directory
cwd = os.getcwd()

# Get the list of files in the current working directory
files = os.listdir(cwd)

# Get the list of files with the .vert.glsl extension
vertFiles = [f for f in files if f.endswith('.vert.glsl')]
print("Vertex Shader Files: ", vertFiles)

# Get the list of files with the .frag.glsl extension
fragFiles = [f for f in files if f.endswith('.frag.glsl')]
print("Fragment Shader Files: ", fragFiles)

# Compile the vertex shaders
for vertFile in vertFiles:
    print("Compiling to SPIR-V: ", vertFile)
    subprocess.run(["glslangValidator", "-S", "vert", vertFile, "--target-env", "vulkan1.0","-o", vertFile.replace(".glsl", ".spv")])

# Compile the fragment shaders
for fragFile in fragFiles:
    print("Compiling to SPIR-V: ", fragFile)
    subprocess.run(["glslangValidator", "-S", "frag", fragFile, "--target-env", "vulkan1.0","-o", fragFile.replace(".glsl", ".spv")])

# Compile .glsl to .wgsl
for vertFile in vertFiles:
    print("Compiling to WGSL: ", vertFile)
    subprocess.run(["naga", vertFile, vertFile.replace(".glsl", ".wgsl")])
    
for fragFile in fragFiles:
    print("Compiling to WGSL: ", fragFile)
    subprocess.run(["naga", fragFile, fragFile.replace(".glsl", ".wgsl")])

# If windows, LF -> CRLF
if sys.platform == 'win32':
    for vertFile in vertFiles:
        with open(vertFile.replace(".glsl", ".wgsl"), 'r') as file:
            data = file.read()
        with open(vertFile.replace(".glsl", ".wgsl"), 'w', newline='\r\n') as file:
            file.write(data)
    for fragFile in fragFiles:
        with open(fragFile.replace(".glsl", ".wgsl"), 'r') as file:
            data = file.read()
        with open(fragFile.replace(".glsl", ".wgsl"), 'w', newline='\r\n') as file:
            file.write(data)
    
print("Done!")
# wait for user input
input("Press Enter to continue...")