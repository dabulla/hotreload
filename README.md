# Hot Reload Server
Minimalistic boilerplate C++-Class and Qml-Template to enable hot reload (inspired by [Flutter](https://flutter.dev/docs/development/tools/hot-reload))
This project can be integrated easily into any C++-Code.
It enables you to compile the applications once and make changes to qml code without recompiling.
If you use the Applicationstate pattern, you can even refresh your Codebase while using the App!
You can even test your app on multiple devices at the same time, e.g. Desktop/Android/iOS devices of different sizes can connect to your
qml directory and show you the interface while you develop it.
This is done with a minimal Http-Server and using Qmls awesome network loading mechanisms.
The C++ Code is less than 300 lines, the Qml-Boilerplate is ~100 lines of code.

# Features
Testing and iterating fast over your qml app is possible using tools like [QmlLive](https://github.com/qtproject/qt-apps-qmllive).
However the most tools are standalone applications and lack of support for custom C++ parts of your application.
For example if you have a custom imageprovider, this cannot be used with most tools.
Additionally this template adds persistence of appstate and testing over network.
Recently it is also possible to cache versions. This means after a successful develpment
session you still have the latest version on your device without a full (apk) deployment.
This mechanism can be used to roll out updates to beta-channels of your application.

# How to use
To quickly test this app/template/boilerplate and have an overview you can use it as a commandlinetool.
However, the main usecase is it's C++ integration into custom apps.
## Command Line Standalone
Hot Reload Server can be used as a commandline tool to quickly test it
```
HotReloadServer[.exe] <qmldir> [startscript]
```
Every subdirectory and file in `qmldir` will be watched for changes. The directory will be made publicly available under http://localhost:8080/v0/.
If `startscript` is added, HotReloadServer will not only start the server, but also start the qml application.
This is for convenience to quickly test small qml apps.
An example is in this repo under /testapp

The Qml Object AppState and AppStatePersistent might futher support your development :D!
Note: At the time of this writing I had quite some trouble to use Qmls `Settings`-Item from labs.
My items use `LocalStorage` instead, which worked better for me on Windows/Android.

## C++ Integration

For the very lowest entry barrier:

Just add the line
```
StatefulHotReloadServer server(dir, skipDirs);
```
to your code and copy the class to your application. I'm working on making it available as a submodule without much hassle.
Use the "main.qml" from testapp as a new entrypoint for `QQmlApplicationEngine::load(...)`. Enter your old root-Qml file in liveReloadRoot.rootFile.

Start your application...


# How does it work

`StatefulHotReloadServer` starts two webservers. The first one delivers your Qml directory via Http. The second server sendes Websocket signals whenever
a file changes. On the Qmlside the root element is reloaded (Url is changed slighly to avoid caching between versions).
Before the Qml element is exchanged, it's property 'appState' is saved and afterwards it is restored.

# How does it work (and more advanced features)

There is a `QNetworkAccessManager` That listens to protocols starting with 'cached'. It then looks to a directory for a cached version
of the requested file while starting a network request. If the network request was successful, the cached file is rewritten. If not, the cached file is returned.
The cache only has one version at this moment and files are never deleted. So it can happen, that multiple versions get mixed in the worst case,
it should work for most common cases.
Optional: There should be a "Versions.qml" in the project root if you want to be able to switch to different
development channels (E.g. github master and github stable versions of your app)
