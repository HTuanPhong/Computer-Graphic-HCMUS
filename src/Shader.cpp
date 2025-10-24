#include "Shader.h"

char* readFile(const char* filename)
{
    std::ifstream fin(filename, std::ios::binary);

    if (fin.is_open())
    {
        fin.seekg(0, std::ios::end);
        long long size = fin.tellg();
        char* content = new char[size + 1];
        fin.seekg(0, std::ios::beg);
        
        fin.read(&content[0], size);
        content[size] = '\0';
        fin.close();
        return content;
    }
    throw std::runtime_error("Failed to open shader file.");
}

Shader::Shader(const char* vertexFile, const char* fragmentFile, const char* fallbackVertexFile, const char* fallbackFragmentFile)
{
    char* vertexSource;
    char* fragmentSource;
    try
    {
        vertexSource = readFile(vertexFile);
        fragmentSource = readFile(fragmentFile);
    }
    catch (...)
    {
        try
        {
            vertexSource = readFile(fallbackVertexFile);
            fragmentSource = readFile(fallbackFragmentFile);
        }
        catch (...)
        {
            throw std::runtime_error("Failed to load shaders.");
        }
        
    }
    
    // Create Vertex Shader Object and get its reference
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // Attach Vertex Shader source to the Vertex Shader Object
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    // Compile the Vertex Shader into machine code
    glCompileShader(vertexShader);
    delete[] vertexSource;

    // Create Fragment Shader Object and get its reference
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Attach Fragment Shader source to the Fragment Shader Object
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    // Compile the Vertex Shader into machine code
    glCompileShader(fragmentShader);
    delete[] fragmentSource;

    // Create Shader Program Object and get its reference
    ID = glCreateProgram();
    // Attach the Vertex and Fragment Shaders to the Shader Program
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    // Wrap-up/Link all the shaders together into the Shader Program
    glLinkProgram(ID);

    // Delete the now useless Vertex and Fragment Shader objects
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::activateProgram()
{
    glUseProgram(ID);
}

void Shader::deleteProgram()
{
    glDeleteProgram(ID);
}