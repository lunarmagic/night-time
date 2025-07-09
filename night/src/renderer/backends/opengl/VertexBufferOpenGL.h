#pragma once

#include "geometry/Vertex.h"
//#include <GL/glew.h>
//#include "GLCall.h"

#define VERTEX_BUFFER_STATIC false
#define VERTEX_BUFFER_DYNAMIC true

namespace night
{

    void __vbo_init(u32& renderer_id, const void* data, u32 size);
    void __vbo_init(u32& renderer_id, u32 size);
    void __vbo_clean(u32& renderer_id);
    void __vbo_data(u32& renderer_id, const void* data, u32 size);
    void* __vbo_begin(u32& renderer_id);
    void __vbo_end();
    void __vbo_bind(u32 const& renderer_id);
    void __vbo_unbind();

	template<typename T>
	struct VertexBufferOpenGL
	{
        VertexBufferOpenGL() = default;

		void init(const void* data, u32 size);
		void init(u32 size);

        void clean();

		void data(const void* data, u32 size);

		void begin();
		void submit(const T& t);
		void end();

		void bind() const;
		void unbind() const;

		s32 count() const { return _count; }

    private:

        u32 _rendererID{ 0 };
        T* _buffer{ nullptr };
        s32 _count{ 0 };
	};

    template<typename T>
    void VertexBufferOpenGL<T>::init(const void* data, u32 size)
    {
        __vbo_init(_rendererID, data, size);
    }

    template<typename T>
    void VertexBufferOpenGL<T>::init(u32 size)
    {
        __vbo_init(_rendererID, size);
    }

    template<typename T>
    void VertexBufferOpenGL<T>::clean()
    {
        //GLCall(glDeleteBuffers(1, &_rendererID));
        __vbo_clean(_rendererID);
        _rendererID = 0;
        _buffer = nullptr;
        _count = 0;
    }

    template<typename T>
    void VertexBufferOpenGL<T>::data(const void* data, u32 size)
    {
        //GLCall(glBindBuffer(GL_ARRAY_BUFFER, _rendererID));
        //GLCall(glBufferData(GL_ARRAY_BUFFER, size * sizeof(T), data, GL_STATIC_DRAW));
        __vbo_data(_rendererID, data, size * sizeof(T));
        //_count = size * 6;
        _count = size;
    }

    template<typename T>
    void VertexBufferOpenGL<T>::begin()
    {
        //GLCall(glBindBuffer(GL_ARRAY_BUFFER, _rendererID));
        //GLCall(_buffer = (T*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
        _buffer = (T*)__vbo_begin(_rendererID);
        _count = 0;
    }

    template<typename T>
    void VertexBufferOpenGL<T>::submit(const T& t)
    {
        *_buffer = t;
        _buffer++;
        _count++;
    }

    template<typename T>
    void VertexBufferOpenGL<T>::end()
    {
        //glUnmapBuffer(GL_ARRAY_BUFFER);
        __vbo_end();
    }

    template<typename T>
    void VertexBufferOpenGL<T>::bind() const
    {
        __vbo_bind(_rendererID);
        /*GLCall(glBindBuffer(GL_ARRAY_BUFFER, _rendererID));*/
    }

    template<typename T>
    void VertexBufferOpenGL<T>::unbind() const
    {
        __vbo_unbind();
        //GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

}