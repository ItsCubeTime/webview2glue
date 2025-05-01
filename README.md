# webview2glue
A Visual Studio 2022 project that produces 2 small dll files webview2glue.dll (~85kb~ 145kb) &amp; WebView2Loader.dll (70kb), which exposes functions for managing webviews in Microsoft Windows (Webview2) without needing the capability of communicating over comtypes. Originally developed for my "Fast Python Library", but can be used by any app capable of loading &amp; communicating with dll files.

Go to the latest release to see a list of all the functions `webview2glue.dll` exposes. Some releases also provide a python file showcasing a subset of `webview2glue.dll`'s functions in use.

Based on https://github.com/MicrosoftEdge/WebView2Samples/tree/main/GettingStartedGuides/Win32_GettingStarted

Also requires https://upx.github.io/ added to your PATH environment variable for the post build step to work (used for dll compression)
