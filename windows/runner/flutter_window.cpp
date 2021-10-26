#include "flutter_window.h"

#include <optional>

#include "flutter/method_channel.h"

#include <thread>

#include "flutter/standard_method_codec.h"
#include "flutter/event_channel.h"
#include "flutter/event_stream_handler_functions.h"

#include "flutter/generated_plugin_registrant.h"

//声明函数
//调用平台方法
void configMethodChannel(flutter::FlutterEngine*);
//向Flutter侧发Event
void configEventChannel(flutter::FlutterEngine*);

//发送Event的方法
std::unique_ptr < flutter::StreamHandlerError<flutter::EncodableValue>> on_listen(const flutter::EncodableValue* arguments,
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events);
//取消Event监听时调用的方法
std::unique_ptr < flutter::StreamHandlerError<flutter::EncodableValue>> on_cancel(const flutter::EncodableValue* arguments);
//开新线程发送消息防止阻塞
void sentEvent(std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events);
FlutterWindow::FlutterWindow(const flutter::DartProject& project)
    : project_(project) {}

FlutterWindow::~FlutterWindow() {}

bool FlutterWindow::OnCreate() {
  if (!Win32Window::OnCreate()) {
    return false;
  }

  RECT frame = GetClientArea();

  // The size here must match the window dimensions to avoid unnecessary surface
  // creation / destruction in the startup path.
  flutter_controller_ = std::make_unique<flutter::FlutterViewController>(
      frame.right - frame.left, frame.bottom - frame.top, project_);
  // Ensure that basic setup of the controller was successful.
  if (!flutter_controller_->engine() || !flutter_controller_->view()) {
    return false;
  }
  RegisterPlugins(flutter_controller_->engine());
  SetChildContent(flutter_controller_->view()->GetNativeWindow());

  configMethodChannel(flutter_controller_->engine());
  configEventChannel(flutter_controller_->engine());
  return true;
}

void FlutterWindow::OnDestroy() {
  if (flutter_controller_) {
    flutter_controller_ = nullptr;
  }

  Win32Window::OnDestroy();
}

LRESULT
FlutterWindow::MessageHandler(HWND hwnd, UINT const message,
                              WPARAM const wparam,
                              LPARAM const lparam) noexcept {
  // Give Flutter, including plugins, an opportunity to handle window messages.
  if (flutter_controller_) {
    std::optional<LRESULT> result =
        flutter_controller_->HandleTopLevelWindowProc(hwnd, message, wparam,
                                                      lparam);
    if (result) {
      return *result;
    }
  }

  switch (message) {
    case WM_FONTCHANGE:
      flutter_controller_->engine()->ReloadSystemFonts();
      break;
  }

  return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}


void configMethodChannel(flutter::FlutterEngine* engine) {
    //在Flutter端创建的Channel名称
    std::string targetChannel = "LogChannel";       
    
    //获得一个解码器的实例
    const flutter::StandardMethodCodec& codec = flutter::StandardMethodCodec::GetInstance();

    //向engine注册Channel
    flutter::MethodChannel method_channel_(engine->messenger(), targetChannel, &codec);



    //设置你的调用方法
    method_channel_.SetMethodCallHandler([](const auto& call, auto result) {
        //compare的内容与Flutter端的invokeMethod对应
        if (call.method_name().compare("logHalo") == 0) {

            std::cout << "Native:Halo!" << std::endl;
            result->Success(flutter::EncodableValue("Call Native Successfully!"));
            //想Flutter发送Error的用法
            //result->Error(flutter::EncodableValue("Call Native Failed!"));

        }
        });
 }

void configEventChannel(flutter::FlutterEngine* engine){
    //Flutter端Channel的名称
    std::string eventChannel = "LogLoopChannel";
    const flutter::StandardMethodCodec& codec = flutter::StandardMethodCodec::GetInstance();
    //向引擎注册
    flutter::EventChannel event_channel_(engine->messenger(), eventChannel, &codec);

    event_channel_.SetStreamHandler(
       std::make_unique<flutter::StreamHandlerFunctions<flutter::EncodableValue>>(on_listen, on_cancel)
    );

}

//向Flutter端发送消息
std::unique_ptr < flutter::StreamHandlerError<flutter::EncodableValue>> on_listen(const flutter::EncodableValue* arguments,
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events) {
    
    //开新线程 防止阻塞
    std::thread t(sentEvent,std::move(events));
    t.detach();

    return NULL;
}

//cancel函数
std::unique_ptr < flutter::StreamHandlerError<flutter::EncodableValue>> on_cancel(const flutter::EncodableValue* arguments) {
    //什么也不做
    return NULL;
}

void sentEvent(std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events) {
    int i = 0;
    std::string str;
    while(i < 100){
        str = std::string("Native:Event ") + std::to_string(i);
        events.get()->Success(flutter::EncodableValue(str));
        //让该线程休眠两秒
        std::this_thread::sleep_for(std::chrono::seconds(2));
        ++i;
    }


}