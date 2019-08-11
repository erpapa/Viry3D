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

#include "Grayscale.h"
#include "graphics/Camera.h"
#include "graphics/Material.h"
#include "graphics/RenderTarget.h"

namespace Viry3D
{
	Grayscale::Grayscale()
	{
		m_material = RefMake<Material>(Shader::Find("PostProcessing/Grayscale"));
	}

	Grayscale::~Grayscale()
	{
		m_material.reset();
	}

	void Grayscale::OnRenderImage(const Ref<RenderTarget>& src, const Ref<RenderTarget>& dst)
	{
		m_material->SetTexture(MaterialProperty::TEXTURE, src->color);
		Camera::Blit(src, dst, m_material);
	}
}
