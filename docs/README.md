# dengine

- Cross platform and lightweight graphics engine.
- Written in C99
- Uses at least OpenGL 2.1 and OpenGL ES 2.0
- Some features need GL 3.2 or ES 3.2 but will be disabled with not avaliable
- It's possible to use GL 2.1 provided the following generic extensions are available:
	- `GL_*_vertex_array_object`
	- `GL_*_framebuffer_object`
	- `GL_*_depth_texture`
- 3D graphics is the main focus

# modules/libs

1. **dengine-core** – Contains the basic functions to create context, textures, buffers and drawing  
	**HEADERS**  
	- dengine_config.h – Compile defines for compiling
	- window.h – Creates context
	- loadgl.h – Function loader for OpenGL
	- checkgl.h – Check for any OpenGL errors
	- buffer.h – Creates general buffers (array, index, unifrom etc.)
	- vertex.h – Vertex attributes and arrays for OpenGL
	- shader.h – Shader compilation and linking
	- texture.h – Loads 2D and Cubemap textures
	- camera.h – Creates view and project matrix for 3D
	- primitive.h – Generate basic primitives e.g. triangles, quad, cube, grid, 3d-axis
	- draw.h – Draws arrays or indexed elements
	- input.h – Interface to native Input
	- framebuffer.h – Framebuffer operations
	- lighting.h – Basic light sources
	- vao.h - Vertex array objects
	- macros.h - Various macro defines
	- renderbuffer.h - Renderbuffer objects
	- dengine.h - Inlines some basic init and terminate functions

	**DEPENDS**  
	- GLFW – Context and Input for Desktop GL
	- CGLM – Math for OpenGL
	- GLAD – Function and extension loader for OpenGL
	- EGL – Context creation for GLES
	- STB – stb_image.h to read texture files, stb_image_write.h to write
      
2. **dengine-utils** – Various misc. utilities  
	**HEADERS**  
	- filesys.h – File functions like read to a memory block
	- os.h – OS specifics like mkdir, dialogs
	- logging.h – Logging functions
	- vtor.h – C++ like std::vector implemetation
	- confserialize.h – Serialize ASCII text to key-value pairs
	- rng.h – Random number generator
	- timer.h – Time interface for delta and current
	- debug.h - Debugging and tracing
	- str.h - String utils

	**DEPENDS**  
	- GTK3 – For dialogs on Linux
                   
3. **dengine-model** – Loads various model formats (*Wavefront OBJ’s*)  
	**HEADERS**  
	- model.h – Loads model formats
                      
4. **dengine-scene** – Scene creation. Serializes JSON  
	**HEADERS**  
	- scene.h – Construct scenes and add entities. Parse scenes to and fro JSON
	- ecs.h - Entity component system definitions

	**DEPENDS**  
	- Any JSON library can do. Use JSON-C
      
5. **dengine-script** – Scripting using Python 3  
	**HEADERS**  
	- scripting.h – Creates an initialize Python Interpreter. Loads .py scripts
	- py_modules.h – Initializes Python modules
	- `<python-modules>` - Extension C modules for Python scripts

	**DEPENDS**  
	- PYTHON 3 – Very nice library :)
6. **dengine-gui** -    Render GUI items immediatly with no hidden state. GUI items are rendered immediately and have no state  
	**HEADERS**  
	- gui.h – All gui init and drawing operations and “widgets”

	**DEPENDS**  
	- STB – stb_truetype.h to read TrueType fonts

# programs(tbd)
1. **dengitor** – Scene editor for dengine-scene and dengine project manager. Incorporates most if not all libraries
2. **dengibuild** – Compile and package a project to a platform  
	**DEPENDS**  
	- A fast compression algo like LZ4
	- Working C compiler. 
	- Working CMake installation
