
#include "nightpch.h"
#include "RenderGraph.h"

namespace night
{
#if 0
    void RenderGraph::current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, TextureUniformData const& texture)
    {
        if (render_target == nullptr)
        {
            render_target = utility::renderer().default_render_target();
            ASSERT(render_target != nullptr);
        }

        _currentRenderTarget = render_target;
        _currentMaterial = material;

        vector<TextureUniformData> s_textures;
        if (texture.texture != nullptr)
        {
            s_textures.push_back(texture);
        }

        // TODO: add textures;
        _currentBuffer = &_sortedBuffers[render_target][material][s_textures];
    }
#endif

    void RenderGraph::current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, vector<TextureUniformData> const& textures)
    {
        if (render_target == nullptr)
        {
            render_target = utility::renderer().default_render_target();
            ASSERT(render_target != nullptr);
        }

        _currentRenderTarget = render_target;
        _currentMaterial = material;

        //vector<TextureUniformData> s_textures;
        //for (const auto& i : textures)
        //{
        //    ASSERT(i.texture != nullptr);
        //    s_textures.push_back(i);
        //}

        _currentBuffer = &_sortedBuffers[render_target][material][textures];
    }

#if 0
    void RenderGraph::current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, handle<const ITexture> texture)
    {
        if (render_target == nullptr)
        {
            render_target = utility::renderer().default_render_target();
            ASSERT(render_target != nullptr);
        }

        _currentRenderTarget = render_target;
        _currentMaterial = material;

        vector<TextureUniformData> s_textures;
        if(texture != nullptr)
        {
            s_textures.push_back(TextureUniformData{ .texture = texture, .sample_depth_buffer = false });
        }
        
        // TODO: add textures;
        _currentBuffer = &_sortedBuffers[render_target][material][s_textures];
    }
#endif

    void RenderGraph::current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, vector<handle<const ITexture>> const& textures)
    {
        if (render_target == nullptr)
        {
            render_target = utility::renderer().default_render_target();
            ASSERT(render_target != nullptr);
        }

        _currentRenderTarget = render_target;
        _currentMaterial = material;

        vector<TextureUniformData> s_textures;
        for (const auto& i : textures)
        {
            ASSERT(i != nullptr);
            shandle<const ITexture> ptr = i.ptr().lock();
            s_textures.push_back(TextureUniformData{ .texture = ptr, .sample_depth_buffer = false });
        }

        _currentBuffer = &_sortedBuffers[render_target][material][s_textures];
    }

    void RenderGraph::current_buffer(handle<const ITexture> render_target, handle<IMaterial> material, std::nullptr_t null)
    {
        if (render_target == nullptr)
        {
            render_target = utility::renderer().default_render_target();
            ASSERT(render_target != nullptr);
        }

        _currentRenderTarget = render_target;
        _currentMaterial = material;

        _currentBuffer = &_sortedBuffers[render_target][material][{}];
    }

}