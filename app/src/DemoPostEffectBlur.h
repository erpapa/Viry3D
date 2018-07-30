/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "Demo.h"
#include "Application.h"
#include "graphics/Display.h"
#include "graphics/Camera.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "graphics/Texture.h"
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"

namespace Viry3D
{
    class DemoPostEffectBlur : public Demo
    {
    public:
        Camera* m_camera;
        Label* m_label;

        void InitPostEffectBlur()
        {
            m_camera->SetDepth(0);
            auto color_texture = Texture::CreateRenderTexture(
                    1280,
                    720,
                    VK_FORMAT_R8G8B8A8_UNORM,
                    true,
                    VK_FILTER_LINEAR,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
            auto depth_texture = Texture::CreateRenderTexture(
                    1280,
                    720,
                    Display::Instance()->ChooseFormatSupported(
                            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT),
                    true,
                    VK_FILTER_LINEAR,
                    VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
            m_camera->SetRenderTarget(color_texture, depth_texture);

            // blur
            int downsample = 1;
            float texel_offset = 1.6f;
            int iter_count = 3;
            float iter_step = 0.0f;

            int width = color_texture->GetWidth();
            int height = color_texture->GetHeight();
            for (int i = 0; i < downsample; ++i)
            {
                width >>= 1;
                height >>= 1;
            }

            auto color_texture_2 = Texture::CreateRenderTexture(
                width,
                height,
                VK_FORMAT_R8G8B8A8_UNORM,
                true,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
            auto color_texture_3 = Texture::CreateRenderTexture(
                width,
                height,
                VK_FORMAT_R8G8B8A8_UNORM,
                true,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

            String vs = R"(
Input(0) vec4 a_pos;
Input(2) vec2 a_uv;

Output(0) vec2 v_uv;

void main()
{
	gl_Position = a_pos;
	v_uv = a_uv;

	vulkan_convert();
}
)";
            String fs = R"(
precision highp float;
      
UniformTexture(0, 0) uniform sampler2D u_texture;

UniformBuffer(0, 1) uniform UniformBuffer01
{
	vec4 u_texel_size;
} buf_0_1;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

const float kernel[7] = float[7]( 0.0205, 0.0855, 0.232, 0.324, 0.232, 0.0855, 0.0205 );

void main()
{
    vec4 c = vec4(0.0);
    for (int i = 0; i < 7; ++i)
    {
        c += texture(u_texture, v_uv + buf_0_1.u_texel_size.xy * float(i - 3)) * kernel[i];
    }
    o_frag = c;
}
)";
            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;

            auto shader = RefMake<Shader>(
                vs,
                Vector<String>(),
                fs,
                Vector<String>(),
                render_state);

            int camera_depth = 2;

            // color -> color2, down sample
            auto blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, color_texture);
            blit_color_camera->SetRenderTarget(color_texture_2, Ref<Texture>());

            for (int i = 0; i < iter_count; ++i)
            {
                // color2 -> color, h blur
                auto material_h = RefMake<Material>(shader);
                material_h->SetVector("u_texel_size", Vector4(1.0f / width * texel_offset * (1.0f + i * iter_step), 0, 0, 0));

                blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, color_texture_2, material_h);
                blit_color_camera->SetRenderTarget(color_texture, Ref<Texture>());

                // color -> color2, v blur
                auto material_v = RefMake<Material>(shader);
                material_v->SetVector("u_texel_size", Vector4(0, 1.0f / height * texel_offset * (1.0f + i * iter_step), 0, 0));

                blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, color_texture, material_v);
                blit_color_camera->SetRenderTarget(color_texture_2, Ref<Texture>());
            }

            // color2 -> color, output
            blit_color_camera = Display::Instance()->CreateBlitCamera(camera_depth++, color_texture_2);
            blit_color_camera->SetRenderTarget(color_texture, Ref<Texture>());

            // color -> window
            Display::Instance()->CreateBlitCamera(0x7fffffff, color_texture);
        }

        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            m_label = label.get();

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::PingFangSC));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            canvas->AddView(label);
        }

        virtual void Init()
        {
            m_camera = Display::Instance()->CreateCamera();

            this->InitPostEffectBlur();
            this->InitUI();
        }

        virtual void Done()
        {
            Display::Instance()->DestroyCamera(m_camera);
            m_camera = nullptr;
        }

        virtual void Update()
        {
            m_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
        }

        virtual void OnResize(int width, int height)
        {

        }
    };
}
