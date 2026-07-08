#pragma once

namespace MiniAppRuntime {

// JavaScript injected at document-start into every mini-app page. Exposes a
// small, stable `MiniApp` API over the backend's raw native channel (WKWebView
// message handler, WebView2 chrome.webview, or a __miniappNative shim).
//
//   MiniApp.invoke("system.getInfo", {})  -> Promise<result>
//   MiniApp.on("someEvent", cb)           -> subscribe to native events
//
// Native replies come back through window.__miniapp._dispatch(json).
inline const char* miniAppSdkJs()
{
    return R"JS(
(function () {
  if (window.MiniApp) { return; }

  function rawSend(text) {
    try {
      if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.miniapp) {
        window.webkit.messageHandlers.miniapp.postMessage(text);
        return;
      }
      if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(text);
        return;
      }
      if (window.__miniappNative && window.__miniappNative.postMessage) {
        window.__miniappNative.postMessage(text);
        return;
      }
      console.warn("[MiniApp] no native channel available");
    } catch (e) {
      console.error("[MiniApp] send failed", e);
    }
  }

  var seq = 1;
  var pending = {};
  var listeners = {};

  var api = {
    invoke: function (method, params) {
      return new Promise(function (resolve, reject) {
        var id = seq++;
        pending[id] = { resolve: resolve, reject: reject };
        rawSend(JSON.stringify({ type: "invoke", id: id, method: method, params: params || {} }));
      });
    },
    on: function (event, cb) {
      (listeners[event] = listeners[event] || []).push(cb);
      return function () {
        var arr = listeners[event] || [];
        var i = arr.indexOf(cb);
        if (i >= 0) { arr.splice(i, 1); }
      };
    }
  };

  var internal = {
    _dispatch: function (msg) {
      if (!msg || typeof msg !== "object") { return; }
      if (msg.type === "callback") {
        var p = pending[msg.id];
        if (!p) { return; }
        delete pending[msg.id];
        if (msg.ok) { p.resolve(msg.result); }
        else { p.reject(msg.error || { message: "unknown error" }); }
      } else if (msg.type === "event") {
        var arr = listeners[msg.event] || [];
        for (var i = 0; i < arr.length; i++) {
          try { arr[i](msg.data); } catch (e) { console.error(e); }
        }
      }
    }
  };

  Object.defineProperty(window, "MiniApp", { value: api });
  Object.defineProperty(window, "__miniapp", { value: internal });
})();
)JS";
}

} // namespace MiniAppRuntime
