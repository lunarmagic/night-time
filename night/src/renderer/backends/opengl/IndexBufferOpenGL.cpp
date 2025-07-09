
#include "nightpch.h"
#include "IndexBufferOpenGL.h"

#include "GLCall.h"
#include "log/log.h"

namespace night
{

    void IndexBufferOpenGL::init(const u32* data, u32 count)
    {
        _count = count;
        ASSERT(sizeof(u32) == sizeof(GLuint));

        GLCall(glGenBuffers(1, &_rendererID));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID));
        GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
    }

    void IndexBufferOpenGL::clean()
    {
        GLCall(glDeleteBuffers(1, &_rendererID));
        _rendererID = 0;
        _count = 0;
    }

    void IndexBufferOpenGL::bind() const
    {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _rendererID));
    }

    void IndexBufferOpenGL::unbind() const
    {
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

}

