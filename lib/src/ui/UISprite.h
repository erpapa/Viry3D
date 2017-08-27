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

#include "UIView.h"
#include "Atlas.h"

namespace Viry3D
{
	struct SpriteType
	{
		enum Enum
		{
			Simple = 0,
			Sliced = 1,
			Tiled = 2,
			Filled = 3
		};
	};

	struct SpriteFillMethod
	{
		enum Enum
		{
			Horizontal = 0,
			Vertical = 1,
			Radial90 = 2,
			Radial180 = 3,
			Radial360 = 4
		};
	};

	struct SpriteFillOrigin180
	{
		enum Enum
		{
			Bottom = 0,
			Top = 2,
			Right = 3
		};
	};

	struct SpriteFillOrigin360
	{
		enum Enum
		{
			Bottom = 0,
			Right = 1,
			Top = 2,
			Left = 3
		};
	};

	struct SpriteFillOrigin90
	{
		enum Enum
		{
			BottomLeft = 0,
			TopLeft = 1,
			TopRight = 2,
			BottomRight = 3
		};
	};

	struct SpriteFillOriginHorizontal
	{
		enum Enum
		{
			Left = 0,
			Right = 1
		};
	};

	struct SpriteFillOriginVertical
	{
		enum Enum
		{
			Bottom = 0,
			Top = 1
		};
	};

	class UISprite: public UIView
	{
		DECLARE_COM_CLASS(UISprite, UIView);
	public:
		void SetAtlas(const Ref<Atlas>& atlas);
		void SetSpriteName(const String& name);
		void SetSpriteType(SpriteType::Enum type);
		void SetFillMethod(SpriteFillMethod::Enum fill_method);
		void SetFillOrigin(int fill_origin);
		void SetFillAmount(float fill_amount);
		void SetFillClockWise(bool fill_clock_wise);
		const Ref<Atlas>& GetAtlas() const { return m_atlas; }
		const String& GetSpriteName() const { return m_sprite_name; }
		SpriteType::Enum GetSpriteType() const { return m_sprite_type; }
		SpriteFillMethod::Enum GetFillMethod() const { return m_fill_method; }
		int GetFillOrigin() const { return m_fill_origin; }
		float GetFillAmount() const { return m_fill_amount; }
		bool GetFillClockWise() const { return m_fill_clock_wise; }
		virtual void FillVertices(Vector<Vector3>& vertices, Vector<Vector2>& uv, Vector<Color>& colors, Vector<unsigned short>& indices);
		virtual void FillMaterial(Ref<Material>& mat);

	protected:
		UISprite();
		void FillVerticesSimple(Vector<Vector3>& vertices, Vector<Vector2>& uv, Vector<Color>& colors, Vector<unsigned short>& indices);

		Ref<Atlas> m_atlas;
		String m_sprite_name;
		SpriteType::Enum m_sprite_type;
		SpriteFillMethod::Enum m_fill_method;
		int m_fill_origin;
		float m_fill_amount;
		bool m_fill_clock_wise;
	};
}
