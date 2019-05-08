# Hot Reload Server
Minimalistic boilerplate C++-Class and Qml-Template to enable hot reload (inspired by [Flutter](https://flutter.dev/docs/development/tools/hot-reload))
This project can be integrated easily into any C++-Code.
It enables you to compile the applications once and make changes to qml code without recompiling.
If you use the Applicationstate pattern, you can even refresh your Codebase while using the App!
You can even test your app on multiple devices at the same time, e.g. Desktop/Android/iOS devices of different sizes can connect to your
qml directory and show you the interface while you develop it.
This is done with a minimal Http-Server and using Qmls awesome network loading mechanisms.
The C++ Code is less than 300 lines, the Qml-Boilerplate is ~100 lines of code.

# Why
Testing and iterating fast over your qml app is possible using tools like [QmlLive](https://github.com/qtproject/qt-apps-qmllive).
However the most tools are standalone applications and lack of support for custom C++ parts of your application.
For example if you have a custom imageprovider, this cannot be used with most tools.
Additionally this template adds persistence of appstate.

# How to use
To quickly test this app/template/boilerplate and have an overview you can use it as a commandlinetool.
However, the main usecase is it's C++ integration into custom apps.
## Command Line Standalone
Hot Reload Server can be used as a commandline tool to quickly test it
```
HotReloadServer[.exe] <qmldir> [startscript]
```
Every subdirectory and file in qmldir will be watched for changes. The directory will be made publicly available under http://localhost:8080/v0/.
If startscript is added, HotReloadServer will not only start the server, but also start the qml application.
This is for convenience to quickly test small qml apps.
An example is in this repo under /testapp

The Qml Object AppState and AppStatePersistent might futher support your development :D!
Note: At the time of this writing I had quite some trouble to use Qmls "Settings"-Item from labs.
My items use "LocalStorage" instead, which worked better for me on Windows/Android.

## C++ Integration

For the very lowest entry barrier:

Just Copy/Paste the class "HotReloadServer" from main.cpp and instantiate it.
Use the "main.qml" from testapp as a new entrypoint for QQmlApplicationEngine::load(...). Enter your old root-Qml file in liveReloadRoot.rootFile.

Start your application...

