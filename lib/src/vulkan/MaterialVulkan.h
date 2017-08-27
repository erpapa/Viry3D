/*
* Viry3D
* Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "Object.h"
#include "vulkan_include.h"
#include "graphics/UniformBuffer.h"

namespace Viry3D
{
	class Texture;

	class MaterialVulkan: public Object
	{
	public:
		virtual ~MaterialVulkan();
		const Vector<VkDescriptorSet>& GetDescriptorSet(int pass_index);

	protected:
		MaterialVulkan();
		void UpdateUniformsBegin(int pass_index);
		void UpdateUniformsEnd(int pass_index);
		void* SetUniformBegin(int pass_index);
		void SetUniformEnd(int pass_index);
		void SetUniform(int pass_index, void* uniform_buffer, const String& name, void* data, int size);
		void SetUniformTexture(int pass_index, const String& name, const Texture* texture);

	private:
		bool CheckWritesDirty(int pass_index) const;

	private:
		Vector<Vector<VkDescriptorSet>> m_descriptor_sets;
		Vector<Ref<UniformBuffer>> m_uniform_buffers;
		Vector<VkWriteDescriptorSet> m_writes_old;
	};
}
