#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glad/glad.h>

#include <iostream>
#include <spdlog/spdlog.h>
#include <fstream>

#define LOG_TRACE(message) spdlog::trace(message)
#define LOG_INFO(message) spdlog::info(message)
#define LOG_WARNING(message) spdlog::warn(message)
#define LOG_ERROR(message) spdlog::error(message)
#define LOG_CRITICAL(message) spdlog::critical(message)

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

unsigned int VAO;
unsigned int VBO;

unsigned int program;
unsigned int vert;
unsigned int frag;

float vertices[] = {
    0.5f,-0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
    0.0f, 0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
   -0.5f,-0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
};

SDL_Window* window = NULL;
SDL_GLContext glContext;

// Compile the shaders and log error if found!
static void compileShader(GLuint shader) 
{
    
}

static size_t getFileSize(const char* path) {
    std::ifstream file;
    file.open(path);
    if (!file.is_open()) {
        LOG_ERROR(fmt::format("Failed to open file: '{}'", path));
        return NULL;
    }

    file.seekg(0, std::ifstream::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ifstream::beg);

    return fileSize;
}

static std::string readFile(const char* path) {
    std::ifstream file;
    file.open(path);
    if (!file.is_open()) {
        LOG_ERROR(fmt::format("Failed to open file: '{}'", path));
        return std::string{};
    }
    
    size_t fileSize = getFileSize(path);

    std::unique_ptr<char[]> buf_content = std::make_unique<char[]>(fileSize);
    file.read(buf_content.get(), fileSize);

    buf_content[fileSize - 1] = '\0';
    std::string strContent{ buf_content.get() };

    file.close();
    return strContent;
}


SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) 
{
    spdlog::set_level(spdlog::level::trace);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SPDLOG_CRITICAL(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    LOG_TRACE("SDL initialized.");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow("MinePlusPlus",
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);
    if (!window) {
        LOG_CRITICAL(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    LOG_TRACE("Window initialized.");

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        LOG_CRITICAL(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    LOG_TRACE("GLContext initialized.");

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SPDLOG_CRITICAL(SDL_GetError());
        return SDL_APP_FAILURE;
    }
    LOG_TRACE("GLAD initialized.");

    SDL_GL_SetSwapInterval(1); // Enable VSync

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.27f, 0.57f, 0.96f, 1.0f);

    const GLubyte* version = glGetString(GL_VERSION);
    std::string versionStr(reinterpret_cast<const char*>(version));
    LOG_INFO(fmt::format("OpenGL version: {}", versionStr));

    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    {
        // Read shader files and get their sizes
        std::string vert_text = readFile("src\\shaders\\triangle.vert");
        std::string frag_text = readFile("src\\shaders\\triangle.frag");

        // Convert shader text to const GLchar*
        const GLchar* c_vert_text = vert_text.c_str();
        const GLchar* c_frag_text = frag_text.c_str();

        // Create shaders
        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

        // Pass the shader source to OpenGL
        glShaderSource(vert, 1, &c_vert_text, nullptr);  // Pass the source code (use nullptr for size)
        glShaderSource(frag, 1, &c_frag_text, nullptr);  // Pass the source code (use nullptr for size)

        // Compile the shaders (assumes shader compilation check is done elsewhere)
        glCompileShader(vert);
        glCompileShader(frag);

        program = glCreateProgram();
        glAttachShader(program, vert);
        glAttachShader(program, frag);
        glLinkProgram(program);

        glDetachShader(program, vert);
        glDetachShader(program, frag);

        glDeleteShader(vert);
        glDeleteShader(frag);
    }

    glEnable(GL_LINE_SMOOTH);
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) 
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    SDL_GL_SwapWindow(window);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) 
{
	switch (event->type) {
	case SDL_EVENT_QUIT: return SDL_APP_SUCCESS;

	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) 
{
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(program);
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
}