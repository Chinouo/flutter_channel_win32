import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key, required this.title}) : super(key: key);

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  //创建一个Channel，参数为Channel的名称
  static const EventChannel logLoopChannel = EventChannel("LogLoopChannel");

  //创建一个Channel，参数为Channel的名称
  static const MethodChannel logChannel = MethodChannel("LogChannel");

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.title),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text("Method Channel"),
            //点击调用平台方法  向控制台输出信息
            MaterialButton(
              color: Colors.lightBlue,
              onPressed: logHalo,
              child: Text("Call Platform Method"),
            ),
            const Text("Event Channel"),
            //点击启动方法 向Flutter端发送信息流
            MaterialButton(
                color: Colors.lightBlue,
                onPressed: reciveEvents,
                child: Text("Recive Event From Platform"))
          ],
        ),
      ),
    );
  }

//接收来自平台的Event
  void reciveEvents() async {
    logLoopChannel.receiveBroadcastStream().listen((event) {
      print(event);
    });
  }

  //调用平台方法向控制台输出Halo
  void logHalo() async {
    try {
      //对应Native的compare函数
      final meg = await logChannel.invokeMethod("logHalo");
      print("$meg");
    } on PlatformException catch (e) {
      print(e);
    }
  }
}
