#include "Shader.h"
#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <sstream>

Shader::Shader(const std::string& filePath) :
   m_FilePath(filePath),
   m_RendererId(0)
{
   ShaderProgramSource source = ParseShader(filePath);
   m_RendererId = CreateShader(source.VertexSource, source.FragmentSource);

   //std::cout << "VertexSource" << std::endl;
   //std::cout << source.VertexSource << std::endl;
   //std::cout << "FragmentSource" << std::endl;
   //std::cout << source.FragmentSource << std::endl;
}

Shader::~Shader()
{
   GLCall(glDeleteProgram(m_RendererId));
}

void Shader::Bind() const
{
   GLCall(glUseProgram(m_RendererId));
}

void Shader::Unbind() const
{
   GLCall(glUseProgram(0));

}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
   GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

ShaderProgramSource Shader::ParseShader(const std::string& filePath)
{
   std::ifstream stream(filePath);

   enum class ShaderType
   {
      NONE = -1,
      VERTEX = 0,
      FRAGMENT = 1
   };

   std::string line;
   std::stringstream ss[2];
   ShaderType type = ShaderType::NONE;

   while (getline(stream, line))
   {
      if (line.find("#shader") != std::string::npos)
      {
         if (line.find("vertex") != std::string::npos)
         {
            type = ShaderType::VERTEX;
         }
         else if (line.find("fragment") != std::string::npos)
         {
            type = ShaderType::FRAGMENT;
         }
      }
      else
      {
         ss[(int)type] << line << '\n';
      }
   }

   return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
   unsigned int id = glCreateShader(type);
   const char* src = source.c_str();
   GLCall(glShaderSource(id, 1, &src, nullptr));
   GLCall(glCompileShader(id));

   int result;
   glGetShaderiv(id, GL_COMPILE_STATUS, &result);
   if (result == GL_FALSE)
   {
      int length;
      GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
      char* message = (char*)alloca(length * sizeof(char));
      GLCall(glGetShaderInfoLog(id, length, &length, message));
      std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
      std::cout << message << std::endl;
      GLCall(glDeleteShader(id));
      return 0;
   }

   return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
   unsigned int program = glCreateProgram();
   unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
   unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

   GLCall(glAttachShader(program, vs));
   GLCall(glAttachShader(program, fs));
   GLCall(glLinkProgram(program));
   GLCall(glValidateProgram(program));

   GLCall(glDeleteShader(vs));
   GLCall(glDeleteShader(fs));

   return program;
}

unsigned int Shader::GetUniformLocation(const std::string& name)
{
   auto found = m_UniformLocationCache.find(name);
   if (found != m_UniformLocationCache.end())
   {
      return found->second;
   }

   GLCall(int location = glGetUniformLocation(m_RendererId, name.c_str()));

   if (location == -1)
   {
      std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
   }

   m_UniformLocationCache[name] = location;
   return location;
}
