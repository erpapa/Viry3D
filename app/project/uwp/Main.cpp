﻿/*
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

#include <wrl.h>
#include <ppltasks.h>

#include "Engine.h"
#include "Debug.h"
#include "Input.h"
#include "container/List.h"
#include "time/Time.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Microsoft::WRL;
using namespace Platform;

static void OnPointerPressed(CoreWindow^ window, PointerEventArgs^ e);
static void OnPointerMoved(CoreWindow^ window, PointerEventArgs^ e);
static void OnPointerReleased(CoreWindow^ window, PointerEventArgs^ e);

static Viry3D::String ConvertString(Platform::String^ src)
{
    unsigned int size = src->Length();
    const wchar_t* data16 = src->Data();
    char32_t* data32 = new char32_t[size + 1];
    for (unsigned int i = 0; i < size; ++i)
    {
        data32[i] = data16[i];
    }
    data32[size] = 0;
    return Viry3D::String(data32);
}

static Platform::String^ ConvertString(const Viry3D::String& src)
{
    auto data32 = src.ToUnicode32();
    int size = data32.Size();
    wchar_t* data16 = new wchar_t[size + 1];
    for (int i = 0; i < size; ++i)
    {
        data16[i] = (wchar_t) data32[i];
    }
    data16[size] = 0;
    return ref new Platform::String(data16);
}

static int g_window_width = 0;
static int g_window_height = 0;

namespace app
{
    ref class App sealed: public IFrameworkView
    {
    public:
        App()
        {
        }

        // IFrameworkView Methods.
        virtual void Initialize(CoreApplicationView^ applicationView)
        {
            // Register event handlers for app lifecycle. This example includes Activated, so that we
            // can make the CoreWindow active and start rendering on the window.
            applicationView->Activated +=
                ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

			CoreApplication::Suspending +=
				ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);
			CoreApplication::Resuming +=
				ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

            // Logic for other event handlers could go here.
            // Information about the Suspending and Resuming event handlers can be found here:
            // http://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh994930.aspx
        }

        virtual void SetWindow(CoreWindow^ window)
        {
            window->SizeChanged +=
                ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);
			window->VisibilityChanged +=
				ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);
            window->Closed +=
                ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

			window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(&OnPointerPressed);
			window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(&OnPointerMoved);
			window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(&OnPointerReleased);

			DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
			currentDisplayInformation->DpiChanged +=
				ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);
			currentDisplayInformation->OrientationChanged +=
				ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);
			DisplayInformation::DisplayContentsInvalidated +=
				ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

			m_window = window;
        }

        virtual void Load(Platform::String^ entryPoint)
        {
            this->RecreateRenderer();
        }

        virtual void Run()
        {
            while (!m_window_closed)
            {
                if (m_window_visible)
                {
                    CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

                    this->Draw();
                }
                else
                {
                    CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
                }
            }

            this->DoneRenderer();
        }

        virtual void Uninitialize()
        {

        }

    private:
        // Application lifecycle event handlers.
        void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
        {
            // Run() won't start until the CoreWindow is activated.
            CoreWindow::GetForCurrentThread()->Activate();
        }

		void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
		{
			// 在请求延期后异步保存应用程序状态。保留延期
			// 表示应用程序正忙于执行挂起操作。
			// 请注意，延期不是无限期的。在大约五秒后，
			// 将强制应用程序退出。
			SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
			deferral->Complete();
		}

		void OnResuming(Platform::Object^ sender, Platform::Object^ args)
		{
			// 还原在挂起时卸载的任何数据或状态。默认情况下，
			// 在从挂起中恢复时，数据和状态会持续保留。请注意，
			// 如果之前已终止应用程序，则不会发生此事件。

			// 在此处插入代码。
		}

        // Window event handlers.
		void OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
		{
			g_window_width = (int) sender->Bounds.Width;
			g_window_height = (int) sender->Bounds.Height;
		}

        void OnVisibilityChanged(CoreWindow^ window, VisibilityChangedEventArgs^ e)
        {
            m_window_visible = e->Visible;
        }

        void OnWindowClosed(CoreWindow^ window, CoreWindowEventArgs^ e)
        {
            m_window_closed = true;
			m_window.Release();
        }

		void OnDpiChanged(DisplayInformation^ sender, Object^ args)
		{
			float dpi = sender->LogicalDpi;
		}

		void OnOrientationChanged(DisplayInformation^ sender, Object^ args)
		{
			auto orientation = sender->CurrentOrientation;
		}

		void OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
		{
			
		}

        void RecreateRenderer()
        {
            this->DoneRenderer();

			Viry3D::String name = "Viry3D";
			Viry3D::String data_path = ConvertString(Package::Current->InstalledLocation->Path);
			Viry3D::String save_path = ConvertString(ApplicationData::Current->LocalFolder->Path);
			data_path = data_path.Replace("\\", "/") + "/Assets";
			save_path = save_path.Replace("\\", "/");

			g_window_width = (int) m_window->Bounds.Width;
			g_window_height = (int) m_window->Bounds.Height;

			void* window = reinterpret_cast<IUnknown*>(m_window.Get());

			m_engine = Viry3D::Engine::Create(window, g_window_width, g_window_height);
			if (m_engine)
			{
				m_engine->SetDataPath(data_path);
				m_engine->SetSavePath(save_path);
			}
        }

        void DoneRenderer()
        {
			if (m_engine)
			{
				Viry3D::Engine::Destroy(&m_engine);
			}
        }

        void Draw()
        {
			if (m_engine)
			{
				if (g_window_width != m_engine->GetWidth() || g_window_height != m_engine->GetHeight())
				{
					void* window = reinterpret_cast<IUnknown*>(m_window.Get());

					m_engine->OnResize(window, g_window_width, g_window_height);
				}
				m_engine->Execute();
			}
        }

	private:
		Agile<CoreWindow> m_window;
        bool m_window_closed = false;
        bool m_window_visible = true;
        bool m_is_glesv3 = false;
		Viry3D::Engine* m_engine = nullptr;
    };
}

// Implementation of the IFrameworkViewSource interface, necessary to run our app.
ref class AppSource sealed : IFrameworkViewSource
{
public:
    virtual IFrameworkView^ CreateView()
    {
        return ref new app::App();
    }
};

// The main function creates an IFrameworkViewSource for our app, and runs the app.
[Platform::MTAThread]
int main(Array<Platform::String^>^)
{
    auto app_source = ref new AppSource();
    CoreApplication::Run(app_source);
    return 0;
}

// Input
using namespace Viry3D;

extern Vector<Touch> g_input_touches;
extern List<Touch> g_input_touch_buffer;
extern bool g_mouse_button_down[3];
extern bool g_mouse_button_up[3];
extern Vector3 g_mouse_position;
extern bool g_mouse_button_held[3];

static bool g_mouse_down = false;

static void OnPointerPressed(CoreWindow^ window, PointerEventArgs^ e)
{
    int x = (int) e->CurrentPoint->Position.X;
    int y = (int) e->CurrentPoint->Position.Y;

    if (!g_mouse_down)
    {
        Touch t;
        t.deltaPosition = Vector2(0, 0);
        t.deltaTime = 0;
        t.fingerId = 0;
        t.phase = TouchPhase::Began;
        t.position = Vector2((float) x, (float) g_window_height - y - 1);
        t.tapCount = 1;
        t.time = Time::GetRealTimeSinceStartup();

        if (!g_input_touches.Empty())
        {
            g_input_touch_buffer.AddLast(t);
        }
        else
        {
            g_input_touches.Add(t);
        }

        g_mouse_down = true;
    }

    g_mouse_button_down[0] = true;
    g_mouse_position.x = (float) x;
    g_mouse_position.y = (float) g_window_height - y - 1;
    g_mouse_button_held[0] = true;
}

static void OnPointerMoved(CoreWindow^ window, PointerEventArgs^ e)
{
    int x = (int) e->CurrentPoint->Position.X;
    int y = (int) e->CurrentPoint->Position.Y;

    if (g_mouse_down)
    {
        Touch t;
        t.deltaPosition = Vector2(0, 0);
        t.deltaTime = 0;
        t.fingerId = 0;
        t.phase = TouchPhase::Moved;
        t.position = Vector2((float) x, (float) g_window_height - y - 1);
        t.tapCount = 1;
        t.time = Time::GetRealTimeSinceStartup();

        if (!g_input_touches.Empty())
        {
            if (g_input_touch_buffer.Empty())
            {
                if (g_input_touches[0].phase == TouchPhase::Moved)
                {
                    g_input_touches[0] = t;
                }
                else
                {
                    g_input_touch_buffer.AddLast(t);
                }
            }
            else
            {
                if (g_input_touch_buffer.Last().phase == TouchPhase::Moved)
                {
                    g_input_touch_buffer.Last() = t;
                }
                else
                {
                    g_input_touch_buffer.AddLast(t);
                }
            }
        }
        else
        {
            g_input_touches.Add(t);
        }
    }

    g_mouse_position.x = (float) x;
    g_mouse_position.y = (float) g_window_height - y - 1;
}

static void OnPointerReleased(CoreWindow^ window, PointerEventArgs^ e)
{
    int x = (int) e->CurrentPoint->Position.X;
    int y = (int) e->CurrentPoint->Position.Y;

    if (g_mouse_down)
    {
        Touch t;
        t.deltaPosition = Vector2(0, 0);
        t.deltaTime = 0;
        t.fingerId = 0;
        t.phase = TouchPhase::Ended;
        t.position = Vector2((float) x, (float) g_window_height - y - 1);
        t.tapCount = 1;
        t.time = Time::GetRealTimeSinceStartup();

        if (!g_input_touches.Empty())
        {
            g_input_touch_buffer.AddLast(t);
        }
        else
        {
            g_input_touches.Add(t);
        }

        g_mouse_down = false;
    }

    g_mouse_button_up[0] = true;
    g_mouse_position.x = (float) x;
    g_mouse_position.y = (float) g_window_height - y - 1;
    g_mouse_button_held[0] = false;
}

namespace Viry3D
{
    static bool GetPathFolder(const String& path, String& folder, String& local_path)
    {
        const String& data_path = Engine::Instance()->GetDataPath();
        const String& save_path = Engine::Instance()->GetSavePath();
        if (path.StartsWith(data_path))
        {
            folder = data_path.Replace("/", "\\");
        }
        else if (path.StartsWith(save_path))
        {
            folder = save_path.Replace("/", "\\");
        }
        else
        {
            Log("path error: %s", path.CString());
            return false;
        }
        local_path = path.Substring(folder.Size() + 1).Replace("/", "\\");

        return true;
    }

    static void CreateFileIfNotExist(const String& path)
    {
        Ref<bool> result;

        String folder;
        String local_path;
        if (!GetPathFolder(path, folder, local_path))
        {
            return;
        }

        Concurrency::create_task(StorageFolder::GetFolderFromPathAsync(ConvertString(folder))).then([&](StorageFolder^ root) {
            return root->CreateFileAsync(ConvertString(local_path), CreationCollisionOption::OpenIfExists);
        }).then([&](StorageFile^ file) {
            result = RefMake<bool>(true);
        });

        while (!result)
        {
            auto window = CoreWindow::GetForCurrentThread();
            if (window)
            {
                window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            }
        }
    }

    bool FileExist(const String& path)
    {
        Ref<bool> exist;

        String folder;
        String local_path;
        if (!GetPathFolder(path, folder, local_path))
        {
            return false;
        }

        Concurrency::create_task(StorageFolder::GetFolderFromPathAsync(ConvertString(folder))).then([&](StorageFolder^ root) {
            return root->TryGetItemAsync(ConvertString(local_path));
        }).then([&](IStorageItem^ item) {
            exist = RefMake<bool>(item != nullptr);
        });

        while (!exist)
        {
            auto window = CoreWindow::GetForCurrentThread();
            if (window)
            {
                window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            }
        }

        return *exist;
    }

    ByteBuffer FileReadAllBytes(const String& path)
    {
        Ref<ByteBuffer> buffer;

        String folder;
        String local_path;
        if (!GetPathFolder(path, folder, local_path))
        {
            return ByteBuffer();
        }

        Concurrency::create_task(StorageFolder::GetFolderFromPathAsync(ConvertString(folder))).then([&](StorageFolder^ root) {
            return root->TryGetItemAsync(ConvertString(local_path));
        }).then([&](IStorageItem^ item) {
            return FileIO::ReadBufferAsync((StorageFile^) item);
        }).then([&](Streams::IBuffer^ ib) {
            auto reader = Streams::DataReader::FromBuffer(ib);
            buffer = RefMake<ByteBuffer>(ib->Length);
            reader->ReadBytes(Platform::ArrayReference<byte>(buffer->Bytes(), buffer->Size()));
        });

        while (!buffer)
        {
            auto window = CoreWindow::GetForCurrentThread();
            if (window)
            {
                window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            }
        }

        return *buffer;
    }

    bool FileWriteAllBytes(const String& path, const ByteBuffer& buffer)
    {
        Ref<bool> result;

        CreateFileIfNotExist(path);

        String folder;
        String local_path;
        if (!GetPathFolder(path, folder, local_path))
        {
            return false;
        }

        Concurrency::create_task(StorageFolder::GetFolderFromPathAsync(ConvertString(folder))).then([&](StorageFolder^ root) {
            return root->TryGetItemAsync(ConvertString(local_path));
        }).then([&](IStorageItem^ item) {
            return FileIO::WriteBytesAsync((StorageFile^) item, Platform::ArrayReference<byte>(buffer.Bytes(), buffer.Size()));
        }).then([&]() {
            result = RefMake<bool>(true);
        });

        while (!result)
        {
            auto window = CoreWindow::GetForCurrentThread();
            if (window)
            {
                window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            }
        }

        return *result;
    }

	void GetCoreWindowSize(void* window, int* width, int* height)
	{
		auto size = reinterpret_cast<CoreWindow^>(window)->Bounds;
		*width = (int) size.Width;
		*height = (int) size.Height;
	}
}
