# webview2glue
A Visual Studio 2022 project that produces 2 small dll files webview2glue.dll (85kb) &amp; WebView2Loader.dll (70kb), which exposes functions for managing webviews in Microsoft Windows (Webview2). Originally developed for my "Fast Python Library", but can be used by any app capable of loading &amp; communicating with dll files.

Based on https://github.com/MicrosoftEdge/WebView2Samples/tree/main/GettingStartedGuides/Win32_GettingStarted

Also requires https://upx.github.io/ added to your PATH environment variable for the post build step to work (used for dll compression)

## Exported functions

`CreateWebView(HWND hWnd)` adds a webview to the window associatedw ith the provided hWnd

`UpdateWebviewBounds(HWND hWnd)` updates the size of the webview to match the size of the window associated with the provided hWnd
