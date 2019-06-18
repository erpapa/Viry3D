/*
* Viry3D
* Copyright 2014-2019 by Stack - stackos@qq.com
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

#pragma once

#include "Component.h"
#include "graphics/Camera.h"
#include "graphics/MeshRenderer.h"
#include "graphics/Skybox.h"
#include "graphics/Light.h"
#include "animation/Animation.h"
#include "ui/CanvasRenderer.h"
#include "ui/Sprite.h"
#include "ui/Label.h"
#include "time/Time.h"
#include "Engine.h"
#include "Resources.h"
#include "Input.h"

namespace Viry3D
{
	class App : public Component
	{
	public:
		Camera* m_camera = nullptr;
		Vector2 m_last_touch_pos;
		Vector3 m_camera_rot = Vector3(5, 180, 0);
		Label* m_fps_label = nullptr;

		App()
		{
			auto cube = Resources::LoadMesh("Library/unity default resources.Cube.mesh");
			auto texture = Resources::LoadTexture("texture/checkflag.png.tex");
			auto cubemap = Resources::LoadTexture("texture/env/prefilter.tex");

			auto material = RefMake<Material>(Shader::Find("Diffuse"));
			material->SetTexture(MaterialProperty::TEXTURE, texture);
			material->SetVector(MaterialProperty::TEXTURE_SCALE_OFFSET, Vector4(40, 20, 0, 0));

			auto camera = GameObject::Create("")->AddComponent<Camera>();
			camera->GetTransform()->SetPosition(Vector3(0, 1, 3));
			camera->GetTransform()->SetRotation(Quaternion::Euler(m_camera_rot));
			camera->SetDepth(0);
			camera->SetCullingMask(1 << 0);
			m_camera = camera.get();

			auto skybox = GameObject::Create("")->AddComponent<Skybox>();
			skybox->SetTexture(cubemap, 0.0f);

			auto floor = GameObject::Create("")->AddComponent<MeshRenderer>();
			floor->GetTransform()->SetPosition(Vector3(0, -5, 0));
			floor->GetTransform()->SetScale(Vector3(20, 10, 10));
			floor->SetMesh(cube);
			floor->SetMaterial(material);
			floor->EnableRecieveShadow(true);

			// auto obj = Resources::LoadGameObject("Resources/res/animations/unitychan/unitychan.go");
			auto obj = Resources::LoadGameObject("Resources/res/model/CandyRockStar/CandyRockStar.go");
			obj->GetTransform()->SetPosition(Vector3(0, 0, 0));
			auto renderers = obj->GetComponentsInChildren<Renderer>();
			for (int i = 0; i < renderers.Size(); ++i)
			{
				renderers[i]->EnableCastShadow(true);
			}
			auto anim = obj->GetComponent<Animation>();
			if (anim)
			{
				anim->Play(0);
			}

			auto ui_camera = GameObject::Create("")->AddComponent<Camera>();
			ui_camera->SetClearFlags(CameraClearFlags::Nothing);
			ui_camera->SetDepth(1);
			ui_camera->SetCullingMask(1 << 1);

			auto canvas = GameObject::Create("")->AddComponent<CanvasRenderer>(FilterMode::Linear);
			canvas->GetGameObject()->SetLayer(1);
			canvas->SetCamera(ui_camera);

			auto label = RefMake<Label>();
			label->SetAlignment(ViewAlignment::Left | ViewAlignment::Top);
			label->SetPivot(Vector2(0, 0));
			label->SetColor(Color(0, 0, 0, 1));
			label->SetTextAlignment(ViewAlignment::Left | ViewAlignment::Top);
			canvas->AddView(label);
			m_fps_label = label.get();
            
			auto light = GameObject::Create("")->AddComponent<Light>();
			light->GetTransform()->SetPosition(Vector3(2.0f, 8.0f, 0));
			light->GetTransform()->SetRotation(Quaternion::Euler(110, 90, 0));
			light->SetColor(Color(1, 1, 1, 1) * 0.7f);
			light->SetType(LightType::Spot);
			light->SetSpotAngle(30);
			light->SetRange(20);
			light->EnableShadow(true);
            light->SetFarClip(20);

			auto light2 = GameObject::Create("")->AddComponent<Light>();
			light2->GetTransform()->SetPosition(Vector3(-2.0f, 8.0f, 0));
			light2->GetTransform()->SetRotation(Quaternion::Euler(70, 90, 0));
			light2->SetColor(Color(1, 1, 1, 1) * 0.7f);
			light2->SetType(LightType::Spot);
			light2->SetSpotAngle(30);
			light2->SetRange(20);
			light2->EnableShadow(true);
            light2->SetFarClip(20);
            
#if 0
			auto blit_camera = GameObject::Create("")->AddComponent<Camera>();
			blit_camera->SetClearFlags(CameraClearFlags::Nothing);
			blit_camera->SetOrthographic(true);
			blit_camera->SetOrthographicSize(1);
			blit_camera->SetNearClip(-1);
			blit_camera->SetFarClip(1);
			blit_camera->SetDepth(2);
			blit_camera->SetCullingMask(1 << 2);

			material = RefMake<Material>(Shader::Find("Unlit/Texture"));
			material->SetTexture(MaterialProperty::TEXTURE, light->GetShadowTexture());
			if (Engine::Instance()->GetBackend() == filament::backend::Backend::OPENGL)
			{
				material->SetVector(MaterialProperty::TEXTURE_SCALE_OFFSET, Vector4(1, -1, 0, 1));
			}

			auto quad = GameObject::Create("")->AddComponent<MeshRenderer>();
			quad->GetGameObject()->SetLayer(2);
			quad->GetTransform()->SetScale(Vector3(1, 1, 1));
			quad->SetMesh(Mesh::GetSharedQuadMesh());
			quad->SetMaterial(material);
#endif
		}

		virtual ~App()
		{
			
		}

		virtual void Update()
		{
			if (m_fps_label)
			{
				m_fps_label->SetText(String::Format("FPS:%d", Time::GetFPS()));
			}

			// camera control
			if (Input::GetTouchCount() > 0)
			{
				const Touch& touch = Input::GetTouch(0);
				if (touch.phase == TouchPhase::Began)
				{
					m_last_touch_pos = touch.position;
				}
				else if (touch.phase == TouchPhase::Moved)
				{
					Vector2 delta = touch.position - m_last_touch_pos;
					m_last_touch_pos = touch.position;

					m_camera_rot.y += delta.x * 0.1f;
					m_camera_rot.x += -delta.y * 0.1f;
					m_camera->GetTransform()->SetRotation(Quaternion::Euler(m_camera_rot));
				}
			}
			if (Input::GetKey(KeyCode::W))
			{
				Vector3 forward = m_camera->GetTransform()->GetForward();
				Vector3 pos = m_camera->GetTransform()->GetPosition() + forward * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
			if (Input::GetKey(KeyCode::S))
			{
				Vector3 forward = m_camera->GetTransform()->GetForward();
				Vector3 pos = m_camera->GetTransform()->GetPosition() - forward * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
			if (Input::GetKey(KeyCode::A))
			{
				Vector3 right = m_camera->GetTransform()->GetRight();
				Vector3 pos = m_camera->GetTransform()->GetPosition() - right * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
			if (Input::GetKey(KeyCode::D))
			{
				Vector3 right = m_camera->GetTransform()->GetRight();
				Vector3 pos = m_camera->GetTransform()->GetPosition() + right * 0.1f;
				m_camera->GetTransform()->SetPosition(pos);
			}
		}
	};
}
