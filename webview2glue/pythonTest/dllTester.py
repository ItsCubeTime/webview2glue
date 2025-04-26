import ctypes
from ctypes import wintypes


import ctypes
from ctypes import wintypes
import os
import uuid
import threading

user32 = ctypes.windll.user32
gdi32 = ctypes.windll.gdi32
kernel32 = ctypes.windll.kernel32

# Fix HANDLE types
wintypes.HCURSOR = wintypes.HANDLE
wintypes.HICON = wintypes.HANDLE
wintypes.HBRUSH = wintypes.HANDLE

class WNDCLASS(ctypes.Structure):
    _fields_ = [
        ("style", wintypes.UINT),
        ("lpfnWndProc", ctypes.WINFUNCTYPE(ctypes.c_long, wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM)),
        ("cbClsExtra", wintypes.INT),
        ("cbWndExtra", wintypes.INT),
        ("hInstance", wintypes.HINSTANCE),
        ("hIcon", wintypes.HICON),
        ("hCursor", wintypes.HCURSOR),
        ("hbrBackground", wintypes.HBRUSH),
        ("lpszMenuName", wintypes.LPCWSTR),
        ("lpszClassName", wintypes.LPCWSTR)
    ]
wintypes.WNDCLASS=WNDCLASS

# Define GUID class
class GUID(ctypes.Structure):
    _fields_ = [
        ("Data1", ctypes.c_uint32),
        ("Data2", ctypes.c_uint16),
        ("Data3", ctypes.c_uint16),
        ("Data4", ctypes.c_ubyte * 8),
    ]

    def __init__(self, guid_string):
        u = uuid.UUID(guid_string)
        super().__init__()
        self.Data1 = u.time_low
        self.Data2 = u.time_mid
        self.Data3 = u.time_hi_version
        self.Data4[:] = u.bytes[8:]

# WebView2 COM interface stubs
class ICoreWebView2Environment(ctypes.Structure):
    pass

class ICoreWebView2Controller(ctypes.Structure):
    pass

class ICoreWebView2EnvironmentCompletedHandler(ctypes.Structure):
    pass

class ICoreWebView2ControllerVtbl(ctypes.Structure):
    _fields_ = [
        ("QueryInterface", ctypes.WINFUNCTYPE(ctypes.HRESULT, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p)),
        ("AddRef", ctypes.WINFUNCTYPE(ctypes.c_ulong, ctypes.c_void_p)),
        ("Release", ctypes.WINFUNCTYPE(ctypes.c_ulong, ctypes.c_void_p)),
        ("get_IsVisible", ctypes.WINFUNCTYPE(ctypes.HRESULT, ctypes.c_void_p, ctypes.POINTER(wintypes.BOOL))),
        ("put_IsVisible", ctypes.WINFUNCTYPE(ctypes.HRESULT, ctypes.c_void_p, wintypes.BOOL)),
        ("get_Bounds", ctypes.WINFUNCTYPE(ctypes.HRESULT, ctypes.c_void_p, ctypes.POINTER(wintypes.RECT))),
        ("put_Bounds", ctypes.WINFUNCTYPE(ctypes.HRESULT, ctypes.c_void_p, wintypes.RECT)),
        ("get_CoreWebView2", ctypes.WINFUNCTYPE(ctypes.HRESULT, ctypes.c_void_p, ctypes.POINTER(ctypes.c_void_p))),
    ]

ICoreWebView2Controller._fields_ = [("lpVtbl", ctypes.POINTER(ICoreWebView2ControllerVtbl))]

# Load WebView2Loader.dll
WebView2Loader = ctypes.windll.LoadLibrary(r'C:\PythonPathLibraries\fast\ui3\WebView2Loader.dll')

CreateCoreWebView2EnvironmentWithOptions = WebView2Loader.CreateCoreWebView2EnvironmentWithOptions
CreateCoreWebView2EnvironmentWithOptions.argtypes = [
    wintypes.LPCWSTR,  # browserExecutableFolder
    wintypes.LPCWSTR,  # userDataFolder
    ctypes.c_void_p,   # ICoreWebView2EnvironmentOptions*
    ctypes.c_void_p    # ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*
]
CreateCoreWebView2EnvironmentWithOptions.restype = ctypes.HRESULT

def format_hresult(hr):
    buf = ctypes.create_unicode_buffer(512)
    kernel32.FormatMessageW(
        0x00001000,  # FORMAT_MESSAGE_FROM_SYSTEM
        None,
        hr,
        0,
        buf,
        len(buf),
        None
    )
    return buf.value

# Handler implementation
class EnvironmentCompletedHandlerVtbl(ctypes.Structure):
    _fields_ = [
        ("QueryInterface", ctypes.WINFUNCTYPE(ctypes.HRESULT, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p)),
        ("AddRef", ctypes.WINFUNCTYPE(ctypes.c_ulong, ctypes.c_void_p)),
        ("Release", ctypes.WINFUNCTYPE(ctypes.c_ulong, ctypes.c_void_p)),
        ("Invoke", ctypes.WINFUNCTYPE(
            ctypes.HRESULT,
            ctypes.c_void_p,
            ctypes.POINTER(ICoreWebView2Environment),
            ctypes.HRESULT
        )),
    ]

class EnvironmentCompletedHandler(ctypes.Structure):
    _fields_ = [("lpVtbl", ctypes.POINTER(EnvironmentCompletedHandlerVtbl))]

    def __init__(self, hwnd):
        super().__init__()
        self.hwnd = hwnd
        self.controller = ctypes.POINTER(ICoreWebView2Controller)()
        self._invoke_cb = self._invoke_wrap()
        self._vtbl = EnvironmentCompletedHandlerVtbl(
            self._query_interface(),
            self._add_ref(),
            self._release(),
            self._invoke_cb
        )
        self.lpVtbl = ctypes.pointer(self._vtbl)

    def _query_interface(self):
        def impl(this, riid, ppv):
            return 0
        return EnvironmentCompletedHandlerVtbl._fields_[0][1](impl)

    def _add_ref(self):
        def impl(this):
            return 1
        return EnvironmentCompletedHandlerVtbl._fields_[1][1](impl)

    def _release(self):
        def impl(this):
            return 1
        return EnvironmentCompletedHandlerVtbl._fields_[2][1](impl)

    def _invoke_wrap(self):
        def invoke(this, env, result_code):
            if result_code != 0:
                print(f"Failed to create environment. HRESULT=0x{result_code:X}: {format_hresult(result_code)}")
                return result_code

            class ControllerHandler(ctypes.Structure):
                pass

            env_obj = ctypes.cast(env, ctypes.POINTER(ICoreWebView2Environment))
            hwnd = self.hwnd

            class CreateControllerFuncType(ctypes.WINFUNCTYPE):
                _restype_ = ctypes.HRESULT
                _argtypes_ = [ctypes.c_void_p, wintypes.HWND, ctypes.POINTER(ctypes.POINTER(ICoreWebView2Controller))]

            # Assumes offset 3 on vtable for CreateCoreWebView2Controller
            controller_ptr = ctypes.POINTER(ICoreWebView2Controller)()
            vtbl = ctypes.cast(env_obj.contents, ctypes.POINTER(ctypes.POINTER(ctypes.c_void_p))).contents
            create_controller_func = ctypes.cast(vtbl[3], CreateControllerFuncType(
                ctypes.HRESULT,
                ctypes.POINTER(ICoreWebView2Environment),
                wintypes.HWND,
                ctypes.POINTER(ctypes.POINTER(ICoreWebView2Controller))
            ))

            hr = create_controller_func(env_obj, hwnd, ctypes.byref(controller_ptr))
            if hr != 0:
                print("Failed to create controller")
                return hr
            self.controller = controller_ptr
            print("WebView2 controller created successfully.")
            return 0

        return EnvironmentCompletedHandlerVtbl._fields_[3][1](invoke)

# PAINT struct
class PAINTSTRUCT(ctypes.Structure):
    _fields_ = [
        ("hdc", wintypes.HDC),
        ("fErase", wintypes.BOOL),
        ("rcPaint", wintypes.RECT),
        ("fRestore", wintypes.BOOL),
        ("fIncUpdate", wintypes.BOOL),
        ("rgbReserved", wintypes.BYTE * 32)
    ]

# Message constants
# import win32con
WM_PAINT = 0x000F
WM_DESTROY = 0x0002
WM_SIZE = 5
WM_NCCALCSIZE = 131
class WINDOWPOS(ctypes.Structure):
    _fields_ = [("hwnd", wintypes.HWND),
                ("hWndInsertAfter", wintypes.HWND),
                ("x", ctypes.c_int),
                ("y", ctypes.c_int),
                ("cx", ctypes.c_int),
                ("cy", ctypes.c_int),
                ("flags", ctypes.c_uint)]
class NCCALCSIZE_PARAMS(ctypes.Structure):
    _fields_ = [
        # ("rgrc", ctypes.POINTER(wintypes.RECT)),  # pointer to RECT struct
        # ("lppos", ctypes.POINTER(WINDOWPOS)),  # pointer to WINDOWPOS struct
        ("rgrc", wintypes.RECT),  # pointer to RECT struct
        ("lppos", WINDOWPOS),  # pointer to WINDOWPOS struct
    ]
@ctypes.WINFUNCTYPE(ctypes.c_long, wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM)
def wnd_proc(hwnd, msg, wparam, lparam):
    if msg == WM_PAINT:
        ps = PAINTSTRUCT()
        hdc = user32.BeginPaint(hwnd, ctypes.byref(ps))
        text = "WebView2 Example (ctypes)"
        gdi32.TextOutW(hdc, 50, 50, text, len(text))
        user32.EndPaint(hwnd, ctypes.byref(ps))
        return 0
    elif msg == WM_NCCALCSIZE:
        sz=ctypes.cast(lparam,ctypes.POINTER(NCCALCSIZE_PARAMS)).contents
        if sz and sz.rgrc:
            sz.rgrc.top -= 30
    elif msg == WM_SIZE:
        myDll.UpdateWebviewBounds(hwnd)
        return 0
    elif msg == WM_DESTROY:
        print("Destroy")
        user32.PostQuitMessage(0)
        return 0
    
    return user32.DefWindowProcW(hwnd, msg, ctypes.c_int64(wparam), ctypes.c_int64(lparam))

# Window creation
def create_window(class_name="MyWindow"):
    hInstance = kernel32.GetModuleHandleW(None)

    wndclass = wintypes.WNDCLASS()
    wndclass.lpfnWndProc = wnd_proc
    wndclass.lpszClassName = class_name
    wndclass.hInstance = hInstance
    wndclass.hCursor = user32.LoadCursorW(None, 32512)  # IDC_ARROW
    wndclass.hbrBackground = 5  # COLOR_WINDOW + 1

    atom = user32.RegisterClassW(ctypes.byref(wndclass))
    hwnd = user32.CreateWindowExW(
        0, class_name, "WebView2 Window",
        0xcf0000, 100, 100, 800, 600,
        None, None, hInstance, None
    )
    user32.ShowWindow(hwnd, 1)
    return hwnd

myDll=ctypes.WinDLL(r"C:\Users\olliv\Desktop\Art And Development\Webview Learning\WebView2Samples\GettingStartedGuides\Win32_GettingStartedDllSimple\x64\Release\WebView2GettingStarted.dll")
def init_webview2(hwnd):
    myDll.CreateWebView.argtypes = [wintypes.HWND]
    myDll.UpdateWebviewBounds.argtypes = [wintypes.HWND]
    myDll.CreateWebView(hwnd)
    # myDll.UpdateWebviewBounds(hwnd)
    # handler = EnvironmentCompletedHandler(hwnd)
    # handler_ptr = ctypes.pointer(handler)
    # result = CreateCoreWebView2EnvironmentWithOptions(None, None, None, handler_ptr)
    # if result != 0:
    #     raise Exception(f"CreateCoreWebView2EnvironmentWithOptions failed: HRESULT={result:X}: {format_hresult(result)}")
    # return handler

# Message loop
def message_loop():
    msg = wintypes.MSG()
    while user32.GetMessageW(ctypes.byref(msg), None, 0, 0) != 0:
        
        user32.TranslateMessage(ctypes.byref(msg))
        user32.DispatchMessageW(ctypes.byref(msg))



def yo():
    hwnd = create_window()
    init_webview2(hwnd)
    message_loop()
import threading
t=threading.Thread(target=yo)
t.start()

# myDll=ctypes.WinDLL(r"C:\Users\olliv\Desktop\Art And Development\Webview Learning\WebView2Samples\GettingStartedGuides\Win32_GettingStartedDllSimple\x64\Release\WebView2GettingStarted.dll")
# myDll.CreateWebView.argtypes = [wintypes.HWND]
# myDll.UpdateWebviewBounds.argtypes = [wintypes.HWND]
# myDll.CreateWebView(hwnd)
# myDll.UpdateWebviewBounds(hwnd)









import time

while True:
    time.sleep(100)