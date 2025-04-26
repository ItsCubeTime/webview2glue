// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include <map>
using namespace Microsoft::WRL;

class WebViewAbstraction{public:
	WebViewAbstraction(ICoreWebView2& webview,ICoreWebView2Controller& webviewController) : 
		                      webview(webview),      webviewController(webviewController)
	{};
	ICoreWebView2& webview; 
	ICoreWebView2Controller& webviewController;
};
//class WebViewAbstraction
//{
//public:
//	WebViewAbstraction(wil::com_ptr<ICoreWebView2> webview, wil::com_ptr<ICoreWebView2Controller> webviewController) :
//		                      webview(webview),       webviewController(webviewController)
//	{};
//	wil::com_ptr<ICoreWebView2> webview;
//	wil::com_ptr<ICoreWebView2Controller> webviewController;
//};
std::map<HWND, WebViewAbstraction>webViews = {};




extern "C" __declspec(dllexport) void CALLBACK CreateWebView(HWND hWnd)
{
	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
				// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
				env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller != nullptr) {
							//ICoreWebView2* webview;
							static wil::com_ptr<ICoreWebView2> webview;
							static wil::com_ptr<ICoreWebView2Controller> webviewController;
							webviewController = controller;
							// Pointer to WebView window
							webviewController->get_CoreWebView2(&webview);
							WebViewAbstraction webViewAbstraction = WebViewAbstraction(*webview,*webviewController);
							webViews.insert(std::pair(hWnd, webViewAbstraction));

							// Add a few settings for the webview
							// The demo step is redundant since the values are the default settings
							wil::com_ptr<ICoreWebView2Settings> settings;
							webview->get_Settings(&settings);
							settings->put_IsScriptEnabled(TRUE);
							settings->put_AreDefaultScriptDialogsEnabled(TRUE);
							settings->put_IsWebMessageEnabled(TRUE);
							//webview->CapturePreview

							// Resize WebView to fit the bounds of the parent window
							RECT bounds;
							GetClientRect(hWnd, &bounds);
							webviewController->put_Bounds(bounds);

							// Schedule an async task to navigate to Bing
							webview->Navigate(L"http://127.0.0.1:90/calendar");
						}


						return S_OK;
					}).Get());
				return S_OK;
			}).Get());
}

extern "C" __declspec(dllexport) void CALLBACK UpdateWebviewBounds(HWND hWnd) {
	RECT bounds;
	GetClientRect(hWnd, &bounds);
	webViews.at(hWnd).webviewController.put_Bounds(bounds);
}

