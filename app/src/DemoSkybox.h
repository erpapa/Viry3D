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
#include "graphics/MeshRenderer.h"
#include "graphics/Mesh.h"
#include "graphics/Texture.h"
#include "math/Quaternion.h"
#include "time/Time.h"
#include "ui/CanvasRenderer.h"
#include "ui/Label.h"
#include "ui/Font.h"

namespace Viry3D
{
    class DemoSkybox : public Demo
    {
    public:
        struct CameraParam
        {
            Vector3 pos;
            Quaternion rot;
            float fov;
            float near_clip;
            float far_clip;
        };
        CameraParam m_camera_param = {
            Vector3(0, 0, 0),
            Quaternion::Identity(),
            45,
            1,
            1000
        };

        Camera* m_camera;
        MeshRenderer* m_renderer_sky;
        Label* m_label;

        void InitSkybox()
        {
            auto cube = Mesh::LoadFromFile(Application::Instance()->GetDataPath() + "/Library/unity default resources.Cube.mesh");

            Vector3 camera_forward = m_camera_param.rot * Vector3(0, 0, 1);
            Vector3 camera_up = m_camera_param.rot * Vector3(0, 1, 0);
            Matrix4x4 view = Matrix4x4::LookTo(m_camera_param.pos, camera_forward, camera_up);
            Matrix4x4 projection = Matrix4x4::Perspective(m_camera_param.fov, m_camera->GetTargetWidth() / (float) m_camera->GetTargetHeight(), m_camera_param.near_clip, m_camera_param.far_clip);

            String vs = R"(
UniformBuffer(0, 0) uniform UniformBuffer00
{
	mat4 u_view_matrix;
	mat4 u_projection_matrix;
} buf_0_0;

UniformBuffer(1, 0) uniform UniformBuffer10
{
	mat4 u_model_matrix;
} buf_1_0;

Input(0) vec4 a_pos;

Output(0) vec3 v_uv;

void main()
{
	gl_Position = (a_pos * buf_1_0.u_model_matrix * buf_0_0.u_view_matrix * buf_0_0.u_projection_matrix).xyww;
	v_uv = a_pos.xyz;

	vulkan_convert();
}
)";
            String fs = R"(
precision highp float;
      
UniformTexture(0, 1) uniform samplerCube u_texture;

Input(0) vec3 v_uv;

Output(0) vec4 o_frag;

void main()
{
    vec4 c = textureLod(u_texture, v_uv, 0.0);
    o_frag = pow(c, vec4(1.0 / 2.2));
}
)";
            RenderState render_state;
            render_state.cull = RenderState::Cull::Front;
            render_state.zWrite = RenderState::ZWrite::Off;
            render_state.queue = (int) RenderState::Queue::Background;
            auto shader = RefMake<Shader>(
                vs,
                Vector<String>(),
                fs,
                Vector<String>(),
                render_state);
            auto material = RefMake<Material>(shader);

            auto renderer = RefMake<MeshRenderer>();
            renderer->SetMaterial(material);
            renderer->SetMesh(cube);
            m_renderer_sky = renderer.get();

            material->SetMatrix("u_view_matrix", view);
            material->SetMatrix("u_projection_matrix", projection);

            Matrix4x4 model = Matrix4x4::Translation(m_camera_param.pos);
            renderer->SetInstanceMatrix("u_model_matrix", model);

            Thread::Task task;
            task.job = []() {
                auto cubemap = Texture::CreateCubemap(1024, VK_FORMAT_R8G8B8A8_UNORM, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, true);
                for (int i = 0; i < cubemap->GetMipmapLevelCount(); ++i)
                {
                    for (int j = 0; j < 6; ++j)
                    {
                        int width;
                        int height;
                        int bpp;
                        ByteBuffer pixels = Texture::LoadImageFromFile(String::Format((Application::Instance()->GetDataPath() + "/texture/prefilter/%d_%d.png").CString(), i, j), width, height, bpp);
                        cubemap->UpdateCubemap(pixels, (CubemapFace) j, i);
                    }
                }
                return cubemap;
            };
            task.complete = [=](const Ref<Thread::Res>& res) {
                material->SetTexture("u_texture", RefCast<Texture>(res));
                m_camera->AddRenderer(renderer);
            };
            Application::Instance()->GetThreadPool()->AddTask(task);
        }

        void InitUI()
        {
            auto canvas = RefMake<CanvasRenderer>();
            m_camera->AddRenderer(canvas);

            auto label = RefMake<Label>();
            canvas->AddView(label);

            label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
            label->SetPivot(Vector2(0, 0));
            label->SetSize(Vector2i(100, 30));
            label->SetOffset(Vector2i(40, 40));
            label->SetFont(Font::GetFont(FontType::PingFangSC));
            label->SetFontSize(28);
            label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);

            m_label = label.get();
        }

        virtual void Init()
        {
            m_camera = Display::Instance()->CreateCamera();

            this->InitSkybox();
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
            Matrix4x4 projection = Matrix4x4::Perspective(m_camera_param.fov, m_camera->GetTargetWidth() / (float) m_camera->GetTargetHeight(), m_camera_param.near_clip, m_camera_param.far_clip);
            m_renderer_sky->GetMaterial()->SetMatrix("u_projection_matrix", projection);
        }
    };
}
