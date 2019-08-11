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

#include "LuaAPI.h"
#include "graphics/Display.h"

namespace Viry3D
{
    class LuaDisplay
    {
    public:
        static void Set(lua_State* L)
        {
            LuaAPI::SetFunction(L, "Display", "CreateCamera", CreateCamera);
            LuaAPI::SetFunction(L, "Display", "DestroyCamera", DestroyCamera);
        }

    private:
        static int CreateCamera(lua_State* L)
        {
            Ref<Camera>* ptr = new Ref<Camera>(Display::Instance()->CreateCamera());
            LuaAPI::PushPtr(L, { ptr, LuaClassType::Camera, LuaPtrType::Shared });
            return 1;
        }

        static int DestroyCamera(lua_State* L)
        {
            LuaClassPtr* p1 = (LuaClassPtr*) lua_touserdata(L, 1);
            Ref<Camera>* camera = (Ref<Camera>*) p1->ptr;
            Display::Instance()->DestroyCamera(*camera);
            return 0;
        }
    };
}
