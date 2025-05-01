// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c
#pragma once
#include <windows.h>
#include <CommCtrl.h> // DefSubclassProc & SetWindowSubclass
//#include <stdlib.h>
//#include <string>
//#include <tchar.h>
#include <wrl.h>
#include <wil/com.h>
#include "WebView2.h"
#include <map>
//#include <mutex> // std::lock
#include <iostream>
#include <functional> // std::function
//#include <atomic>
//#include <chrono> // milliseconds
//#include <thread> // sleep_for
using namespace Microsoft::WRL;

//class WebViewAbstraction{public:
//	WebViewAbstraction(ICoreWebView2* webview,ICoreWebView2Controller* webviewController) ://, EventRegistrationToken eventRegistrationToken) :
//		                      webview(webview),      webviewController(webviewController) //, eventRegistrationToken(eventRegistrationToken)
//	{};
//	WebViewAbstraction() {};
//	ICoreWebView2* webview; 
//	ICoreWebView2Controller* webviewController;
//	//EventRegistrationToken eventRegistrationToken;
//	std::mutex resizeLock;
//};
typedef void (__cdecl *Callable)(LPCWSTR);
class WebViewAbstraction
{
	
public:
	WebViewAbstraction() { messageCb = nullptr; webview = wil::com_ptr<ICoreWebView2>(); wil::com_ptr<ICoreWebView2Controller>(); };
	//WebViewAbstraction(wil::com_ptr<ICoreWebView2> webview, wil::com_ptr<ICoreWebView2Controller> webviewController) :
	//	                      webview(webview),       webviewController(webviewController)//, isResizing(false)//, resizeLock(std::unique_lock<std::mutex>()), conditionVariable(std::make_unique<std::condition_variable>())
	//{
	//	messageCb = nullptr;
		//std::atomic<int> resizeLock{}
		//resizeLock = std::make_shared<std::mutex>();
	//};
	//WebViewAbstraction() {};
	wil::com_ptr<ICoreWebView2> webview;
	wil::com_ptr<ICoreWebView2Controller> webviewController;
	//std::atomic<int> resizeLock{};
	//std::mutex resizeLock;
	//std::unique_ptr<std::mutex> resizeLock;
	//std::unique_lock<std::mutex> resizeLock;
	//std::unique_ptr<std::condition_variable> conditionVariable;
	std::atomic<Callable> messageCb{ nullptr };
	//std::atomic<bool> isResizing{false};
	//std::atomic<bool> hasFinishedLoadingWebview{false};
	//bool isResizing{ false };
	//bool resizeComplete;
};
std::map<HWND, std::shared_ptr<WebViewAbstraction>>webViews = {};


LRESULT CALLBACK MySubclassProc(
	HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if (uMsg == WM_NCDESTROY)
	{
		webViews.erase(hWnd);
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


//extern "C" __declspec(dllexport) ICoreWebView2* CALLBACK getWebview(HWND hWnd) {
//	return &*webViews.at(hWnd).get()->webview;
//}
//extern "C" __declspec(dllexport) ICoreWebView2Controller* CALLBACK getWebviewController(HWND hWnd)
//{
//	//webViews.at(hWnd).get()->webviewController-
//	return &*webViews.at(hWnd).get()->webviewController; 
//}§
 
extern "C" __declspec(dllexport) void __cdecl registerMessageCb(HWND hWnd, Callable cb)
{
	if (webViews.contains(hWnd)) {
		//std::cout << "RegisterMessageCb: Registering cb for handle";
		auto w = webViews.at(hWnd).get();
		w->messageCb = cb;
	}
	else
	{
		std::cout << "RegisterMessageCb: Invalid hWnd" << std::endl;
		std::cout << hWnd << std::endl;
		for (std::map<HWND, std::shared_ptr<WebViewAbstraction>>::iterator it = webViews.begin(); it != webViews.end(); ++it) {
			std::cout << it->first << std::endl;
		}
		/*for (let key : webViews.begin())
		{
		std::cout <<  << std::endl;
		}*/


	}
}

extern "C" __declspec(dllexport) void __cdecl addPermanentJavascript(HWND hWnd, LPCWSTR javascript)
{
	auto w = webViews.at(hWnd).get()->webview;
	w->AddScriptToExecuteOnDocumentCreated(javascript, nullptr);
	w->ExecuteScript(javascript, nullptr);
	
}

extern "C" __declspec(dllexport) void __cdecl executePermanentJavascript(HWND hWnd, LPCWSTR javascript)
{
	webViews.at(hWnd).get()->webview->ExecuteScript(javascript, nullptr);
}

extern "C" __declspec(dllexport) void __cdecl reload(HWND hWnd)
{
	webViews.at(hWnd).get()->webview->ExecuteScript(L"location.reload();", nullptr);
}

extern "C" __declspec(dllexport) void __cdecl runJavascript(HWND hWnd, LPCWSTR javascript)
{
	webViews.at(hWnd).get()->webview->ExecuteScript(javascript, nullptr);
}
class WindowCloseRequestedHandler
	: public RuntimeClass<RuntimeClassFlags<ClassicCom>,
	ICoreWebView2WindowCloseRequestedEventHandler>
{
public:
	//void (*cb)();
	std::function<void()> cb;
	STDMETHODIMP Invoke(ICoreWebView2* sender, IUnknown* args) override
	{
		cb();
		//MessageBox(nullptr, L"Window close requested", L"Event", MB_OK);

		return S_OK;
	}
};
//class LambdaWindowCloseHandler :
//    public RuntimeClass<RuntimeClassFlags<ClassicCom>, ICoreWebView2WindowCloseRequestedEventHandler>
//{
//public:
//    LambdaWindowCloseHandler(std::function<void(ICoreWebView2* sender, IUnknown* args)> callback)
//        : m_callback(std::move(callback)) {}
//
//    STDMETHODIMP Invoke(ICoreWebView2* sender, IUnknown* args) override {
//        if (m_callback) {
//            m_callback(sender, args);
//        }
//        return S_OK;
//    }
//
//private:
//    std::function<void(ICoreWebView2*, IUnknown*)> m_callback;
//};

typedef void(__cdecl *CallableVoid)();
extern "C" __declspec(dllexport) void CALLBACK createWebView(HWND hWnd, LPCWSTR url, CallableVoid loadingFinishedCb)
{
	//WCHAR wideString[4096];       // Buffer for the wide string
	//MultiByteToWideChar(
	//	CP_ACP,             // Code page (ACP = ANSI code page)
	//	0,                  // Flags
	//	url,         // Input string
	//	-1,                 // Input length (-1 = null-terminated)
	//	wideString,         // Output buffer
	//	4096                 // Size of output buffer (in wchar_t)
	//);
	webViews.emplace(hWnd, std::make_shared<WebViewAbstraction>());
	WebViewAbstraction* webViewAbstraction = webViews.at(hWnd).get();
	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[hWnd, url, webViewAbstraction, loadingFinishedCb](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
				// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
				env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[hWnd, url, webViewAbstraction, loadingFinishedCb](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller != nullptr) {
							//ICoreWebView2* webview;
							static wil::com_ptr<ICoreWebView2> webview;
							static wil::com_ptr<ICoreWebView2Controller> webviewController;
							webviewController = controller;
							// Pointer to WebView window
							webviewController->get_CoreWebView2(&webview);
							EventRegistrationToken token;
							//WebViewAbstraction webViewAbstraction = WebViewAbstraction(webview, webviewController);
							//webViews.emplace(hWnd, WebViewAbstraction{ webview, webviewController });
							//webViews.emplace(std::make_pair<HWND, WebViewAbstraction>(hWnd, { webview, webviewController }));
							//webViews.insert(std::make_pair<HWND, WebViewAbstraction>(hWnd, { webview, webviewController }));
							//webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
							//	[webViewAbstraction](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
							webViewAbstraction->webview = webview;
							webViewAbstraction->webviewController = webviewController;

							auto _handler = Make<WindowCloseRequestedHandler>();
							//auto c = (([hWnd]() { webViews.erase(hWnd); }));
							//auto c = std::shared_ptr<void>(([hWnd]() { webViews.erase(hWnd); }));
							//[]()->void c = (([hWnd]() { webViews.erase(hWnd); }));
							//_handler->cb =
							_handler->cb=std::function<void()>([hWnd]() { webViews.erase(hWnd); });
							//function<void(*)> ab = make::function([hWnd]() { webViews.erase(hWnd); });
							//auto a = ([hWnd]() { webViews.erase(hWnd); }); 
							ComPtr<ICoreWebView2WindowCloseRequestedEventHandler> handler =
								_handler;
							//EventRegistrationToken token;
							HRESULT hr = webview->add_WindowCloseRequested(handler.Get(), &token);
							 
							 
							//auto handler = Make<LambdaWindowCloseHandler>(
							//	[appName, shutdownCode](ICoreWebView2* sender, IUnknown* args)
							//{
							//	std::wstringstream ss;
							//	ss << L"[" << appName << L"] Window is closing. Code: " << shutdownCode;
							//	MessageBox(nullptr, ss.str().c_str(), L"Close Event", MB_OK);
							//}
							//);

							//EventRegistrationToken token;

							//{
							//	wil::unique_cotaskmem_string message;
							//	args->TryGetWebMessageAsString(&message);
							//	// processMessage(&message);
							//	PWSTR m = message.get();
							//	if (wcscmp(m, L"___RESIZECOMPLETE") == 0)
							//	{   // Strings are equal
							//		webViewAbstraction->isResizing = false;
							//		webViewAbstraction->conditionVariable->notify_one();
							//	}
							//	webview->PostWebMessageAsString(m); 
							//	return S_OK;
							//}).Get(), &token);
							//webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
							//	[webViewAbstraction](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
							//{
							//	wil::unique_cotaskmem_string message;
							//	args->TryGetWebMessageAsString(&message);

							//	PWSTR m = message.get();
							//	std::cout << "Received message" << std::endl;
							//	std::cout << m << std::endl;
							//	if (wcscmp(m, L"___RESIZECOMPLETE") == 0)
							//	{
							//		{
							//			webViewAbstraction->resizeLock.lock();
							//			webViewAbstraction->resizeComplete = true; // Set flag.
							//			webViewAbstraction->resizeLock.unlock();
							//		}
							//		webViewAbstraction->conditionVariable->notify_one();
							//	}

							//	return S_OK;
							//}).Get(), &token);
							//webViews.insert(std::pair<HWND, WebViewAbstraction>( hWnd, webViewAbstraction ) );

							
							// Add a few settings for the webview
							// The demo step is redundant since the values are the default settings
							wil::com_ptr<ICoreWebView2Settings> settings;
							webview->get_Settings(&settings);
							settings->put_IsScriptEnabled(TRUE);
							settings->put_AreDefaultScriptDialogsEnabled(TRUE);
							settings->put_IsWebMessageEnabled(TRUE);

							//webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
							//	[webViewAbstraction](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
							//{
							//	wil::unique_cotaskmem_string message;
							//	args->TryGetWebMessageAsString(&message);
							//	PWSTR m = message.get();
							//	std::cout << "Received message" << std::endl;
							//	if (wcscmp(m, L"___RESIZECOMPLETE") == 0)
							//	{   // Strings are equal
							//		std::cout << "Received message2" << std::endl;
							//		webViewAbstraction->isResizing = false;
							//	}
							//	webview->PostWebMessageAsString(m);
							//	return S_OK;
							//}).Get(), &token);
							//webview->CapturePreview
							// Resize WebView to fit the bounds of the parent window

							//webview->AddScriptToExecuteOnDocumentCreated
//							(
//								LR"_(window.addEventListener('resize', function(event) { console.log('hey there!'); window.chrome.webview.postMessage(`${window.outerWidth}:${window.outerHeight}`); }, true);
//window.addEventListener('mousedown', function (event) {
//    /** @type {HTMLElement} */
//    let target = event.target;
//    while (target) {
//        for (let cls of target.classList) {
//            if (cls == 'windowDragHandle') {
//                window.chrome.webview.postMessage(`windowDragHandle`);
//                return;
//            }
//        }
//        target = target.parentElement;
//    }
//});
//
//)_",
//								nullptr);
							webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
								[webViewAbstraction,hWnd](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
							{
								wil::unique_cotaskmem_string message;
								args->TryGetWebMessageAsString(&message);
								// processMessage(&message);
								PWSTR m=message.get();
								//if (wcscmp(m, L"windowDragHandle") == 0) {   
								//	// Strings are equal
								//	SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(0, 0));
								//}
								//else {
									if (webViewAbstraction->messageCb) { (*(webViewAbstraction->messageCb))(m); }
								//}
								std::wcout << "received: " << m << std::endl;
								return S_OK;
							}).Get(), &token);

							RECT bounds;
							GetClientRect(hWnd, &bounds);
							webviewController->put_Bounds(bounds);
							// Schedule an async task to navigate to Bing
							//ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler* scriptCompleteHandler= &ICoreWebView2AddScriptToExecuteOnDocumentCreatedCompletedHandler();
							//webview->AddScriptToExecuteOnDocumentCreated(L"document.innerHTML=`Loading`",nullptr);
							//MessageBoxA(nullptr, url, "From C++", MB_OK);
							//MessageBoxW(nullptr, url, L"From C++", MB_OK);

							webview->Navigate(url); // L"http://127.0.0.1:90/calendar";

							//SetWindowSubclass(hWnd, MySubclassProc, 1, 0);
							//webViewAbstraction->hasFinishedLoadingWebview = true;
							if (loadingFinishedCb) { loadingFinishedCb(); }
						}
						

						return S_OK;
					}).Get());
				return S_OK;
			}).Get());
			//while (!webViewAbstraction->hasFinishedLoadingWebview)
			//{
			//	std::this_thread::sleep_for(std::chrono::milliseconds(1));
			//}
}

extern "C" __declspec(dllexport) void CALLBACK setUrl(HWND hWnd, LPCWSTR url)
{
	webViews.at(hWnd).get()->webview->Navigate(url); // L"http://127.0.0.1:90/calendar";
}

//extern "C" __declspec(dllexport) void CALLBACK UpdateWebviewBounds(HWND hWnd) {
//	RECT bounds;
//	GetClientRect(hWnd, &bounds); 
//	WebViewAbstraction* webview = &webViews.at(hWnd);webview->webviewController->put_Bounds(bounds);
//	webview->resizeLock.lock();
//	webview->isResizing = true;
//	webview->webview->PostWebMessageAsString(L"___RESIZECOMPLETE");
//	//webview->conditionVariable->wait(webview->resizeLock);
//	webview->conditionVariable->wait(webview->resizeLock, [&]
//	{
//		return !webview->isResizing;
//	});
//	webview->resizeLock.unlock();
//}

//extern "C" __declspec(dllexport) void CALLBACK UpdateWebviewBounds(HWND hWnd)
//{
//	RECT bounds;
//	GetClientRect(hWnd, &bounds);
//
//	WebViewAbstraction* webview = &webViews.at(hWnd);
//
//	std::cout << "l1" << std::endl;
//	webview->resizeLock.lock(); // Lock before.
//	std::cout << "l2" << std::endl;
//
//	webview->resizeComplete = false; // <-- a new flag you must track.
//	webview->webviewController->put_Bounds(bounds);
//	std::cout << "l3" << std::endl;
//	webview->webview->PostWebMessageAsString(L"___RESIZECOMPLETE");
//	std::cout << "l4" << std::endl;
//	 
//	webview->conditionVariable->wait(webview->resizeLock, [&]
//	{
//		return webview->resizeComplete;
//	});
//	std::cout << "l5" << std::endl;
//	webview->resizeLock.unlock();
//	// Lock automatically released here when `lock` goes out of scope.
//}
//void _UpdateWebviewBounds_waitForUpdate(WebViewAbstraction* webview)
//{
//	webview->webview->ExecuteScript(L"window.document.URL;", Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
//		[webview](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT
//	{
//		LPCWSTR URL = resultObjectAsJson;
//		//doSomethingWithURL(URL);
//		std::cout << "executeScriptCb" << std::endl;
//		webview->isResizing = false;
//		return S_OK;
//	}).Get());
//}


extern "C" __declspec(dllexport) void CALLBACK updateWebviewBounds(HWND hWnd, RECT* newBounds)
{

	
	WebViewAbstraction* webview = webViews.at(hWnd).get();
	//std::cout << "l1" << std::endl;
	//webview->isResizing=true;
	//std::cout << "l2" << std::endl;
	if (newBounds)
	{
		webview->webviewController->put_Bounds(*newBounds);
	}
	else
	{
		RECT bounds;
		GetClientRect(hWnd, &bounds);
		webview->webviewController->put_Bounds(bounds);
	}
	webview->webviewController->NotifyParentWindowPositionChanged();
	//std::cout << "l3" << std::endl;
	//webview->webview->PostWebMessageAsString(L"___RESIZECOMPLETE");
	//webview->webview->
	
	//webview->webview->ExecuteScript(L"`${window.outerWidth}:${window.outerHeight}`;", Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
	//	[webview, boundsUpdatedPostCb](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT
	//{
	//	LPCWSTR URL = resultObjectAsJson;
	//	//doSomethingWithURL(URL);
	//	std::cout << "executeScriptCb" << std::endl;
	//	std::wcout << URL << std::endl;
	//	if (boundsUpdatedPostCb) { boundsUpdatedPostCb(URL); }
	//	webview->isResizing = false;
	//	return S_OK;
	//}).Get());

	//if (boundsUpdatedPostCb) { boundsUpdatedPostCb(URL); }

	//std::thread myThread(_UpdateWebviewBounds_waitForUpdate,webview);
	//myThread.join();
	//std::cout << "l4" << std::endl;

	//webview->isResizing.wait(false);
	//while (true)
	//{
	//	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	//	if (webview->isResizing == false) { break; }
	//}
	//std::cout << "l5" << std::endl;
	// Lock automatically released here when `lock` goes out of scope.
}