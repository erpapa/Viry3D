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

#include "Object.h"
#include "math/Recti.h"
#include "math/Vector4.h"
#include "container/Map.h"

namespace Viry3D
{
    class Texture;

    class SpriteAtlas : public Object
    {
    public:
        struct Sprite
        {
            String name;
            Recti rect;
            Vector4 border;
        };

        static Ref<SpriteAtlas> LoadFromFile(const String& file);
        SpriteAtlas();
        virtual ~SpriteAtlas();
        const Ref<Texture>& GetTexture() const { return m_texture; }
        Vector<String> GetSpriteNames() const { return m_sprite_names; }
        const Sprite& GetSprite(const String& name) const { return m_sprites[name]; }
        const String& GetFilePath() const { return m_file_path; }

    private:
        Ref<Texture> m_texture;
        Map<String, Sprite> m_sprites;
        Vector<String> m_sprite_names;
        String m_file_path;
    };
}
