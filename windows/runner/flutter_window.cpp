#include "flutter_window.h"

#include <optional>

#include "flutter/method_channel.h"

#include <thread>

#include "flutter/standard_method_codec.h"
#include "flutter/event_channel.h"
#include "flutter/event_stream_handler_functions.h"

#include "flutter/generated_plugin_registrant.h"

//��������
//����ƽ̨����
void configMethodChannel(flutter::FlutterEngine*);
//��Flutter�෢Event
void configEventChannel(flutter::FlutterEngine*);

//����Event�ķ���
std::unique_ptr < flutter::StreamHandlerError<flutter::EncodableValue>> on_listen(const flutter::EncodableValue* arguments,
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events);
//ȡ��Event����ʱ���õķ���
std::unique_ptr < flutter::StreamHandlerError<flutter::EncodableValue>> on_cancel(const flutter::EncodableValue* arguments);
//�����̷߳�����Ϣ��ֹ����
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
    //��Flutter�˴�����Channel����
    std::string targetChannel = "LogChannel";       
    
    //���һ����������ʵ��
    const flutter::StandardMethodCodec& codec = flutter::StandardMethodCodec::GetInstance();

    //��engineע��Channel
    flutter::MethodChannel method_channel_(engine->messenger(), targetChannel, &codec);



    //������ĵ��÷���
    method_channel_.SetMethodCallHandler([](const auto& call, auto result) {
        //compare��������Flutter�˵�invokeMethod��Ӧ
        if (call.method_name().compare("logHalo") == 0) {

            std::cout << "Native:Halo!" << std::endl;
            result->Success(flutter::EncodableValue("Call Native Successfully!"));
            //��Flutter����Error���÷�
            //result->Error(flutter::EncodableValue("Call Native Failed!"));

        }
        });
 }

void configEventChannel(flutter::FlutterEngine* engine){
    //Flutter��Channel������
    std::string eventChannel = "LogLoopChannel";
    const flutter::StandardMethodCodec& codec = flutter::StandardMethodCodec::GetInstance();
    //������ע��
    flutter::EventChannel event_channel_(engine->messenger(), eventChannel, &codec);

    event_channel_.SetStreamHandler(
       std::make_unique<flutter::StreamHandlerFunctions<flutter::EncodableValue>>(on_listen, on_cancel)
    );

}

//��Flutter�˷�����Ϣ
std::unique_ptr < flutter::StreamHandlerError<flutter::EncodableValue>> on_listen(const flutter::EncodableValue* arguments,
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events) {
    
    //�����߳� ��ֹ����
    std::thread t(sentEvent,std::move(events));
    t.detach();

    return NULL;
}

//cancel����
std::unique_ptr < flutter::StreamHandlerError<flutter::EncodableValue>> on_cancel(const flutter::EncodableValue* arguments) {
    //ʲôҲ����
    return NULL;
}

void sentEvent(std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events) {
    int i = 0;
    std::string str;
    while(i < 100){
        str = std::string("Native:Event ") + std::to_string(i);
        events.get()->Success(flutter::EncodableValue(str));
        //�ø��߳���������
        std::this_thread::sleep_for(std::chrono::seconds(2));
        ++i;
    }


}