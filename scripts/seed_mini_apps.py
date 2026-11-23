"""Preinstall four offline Mini Apps for manual runtime checks.

Close the host application before running this script, then start it again.
Packages are discovered at startup; no database or first launch is required.
Requires Python 3.9+ and its standard library only.

Usage:
  python scripts/seed_mini_apps.py
  python scripts/seed_mini_apps.py --release
  python scripts/seed_mini_apps.py --packages-dir /path/to/mini_app/packages
  python scripts/seed_mini_apps.py --dry-run

Samples:
  JSON Workbench: local CSS/JS/SVG, JSON editing, errors and nested-page links.
  Runtime Inspector: real system.getInfo calls, concurrent calls and errors.
  Notes A / Notes B: browser localStorage availability, reopen and isolation.

Existing package directories are skipped, including manually modified samples.
To recreate a sample, uninstall that sample in the host and run this script again.
The script never removes packages or changes storage/cache directories.
"""

from __future__ import annotations

import argparse
import html
import json
import os
import platform
import sys
import tempfile
from pathlib import Path


APPS = (
    ("seed.json-workbench", "Test - JSON Workbench", "JSON", "json",
     "Offline JSON editing, local assets and multi-page navigation."),
    ("seed.runtime-inspector", "Test - Runtime Inspector", "API", "runtime",
     "Inspect the MiniApp SDK and exercise real native bridge replies."),
    ("seed.notes-a", "Test - Notes A", "A", "notes",
     "Save a note and compare persistence and isolation with Notes B."),
    ("seed.notes-b", "Test - Notes B", "B", "notes",
     "Use the same storage key as Notes A to check app isolation."),
)

STYLE = """
:root { color-scheme: light dark; --seed-css: ready; --bg: #f3f6f4;
  --panel: #ffffff; --ink: #18251e; --line: #c2d1c8; --accent: #087854; }
@media (prefers-color-scheme: dark) {
  :root { --bg: #18201c; --panel: #242e28; --ink: #edf5ef;
    --line: #5c7365; --accent: #178564; }
}
* { box-sizing: border-box; }
body { max-width: 900px; margin: auto; padding: 24px; background: var(--bg);
  color: var(--ink); font: 15px/1.6 system-ui, sans-serif; }
header { display: flex; align-items: center; gap: 16px; }
header img { width: 52px; height: 52px; }
h1 { font-size: 25px; line-height: 1.3; margin: 0; }
h2 { font-size: 18px; }
section { background: var(--panel); border: 1px solid var(--line);
  border-radius: 12px; margin-top: 20px; padding: 18px; }
button, textarea { font: inherit; }
button { background: var(--accent); color: white; border: 0;
  border-radius: 7px; padding: 9px 14px; cursor: pointer; }
button:disabled { opacity: .6; cursor: wait; }
button:focus-visible, a:focus-visible, textarea:focus-visible {
  outline: 3px solid #41b9e7; outline-offset: 3px; }
.actions { display: flex; flex-wrap: wrap; gap: 10px; margin: 12px 0; }
textarea { display: block; width: 100%; min-height: 160px; padding: 10px;
  color: var(--ink); background: var(--bg); border: 1px solid var(--line);
  border-radius: 6px; resize: vertical; }
a { color: var(--ink); text-underline-offset: 3px; }
pre { white-space: pre-wrap; overflow-wrap: anywhere; margin: 0; }
[data-ok="false"] { color: #df655f; }
code { overflow-wrap: anywhere; }
"""

JAVASCRIPT = r"""
(function () {
  "use strict";
  const byId = id => document.getElementById(id);
  const noteKey = "seed-mini-app-shared-note-key"; // Deliberately identical in A and B.
  const prefix = document.body.dataset.assetPrefix;
  function report(value, ok = true) {
    byId("result").textContent = typeof value === "string" ? value : JSON.stringify(value, null, 2);
    byId("result").dataset.ok = String(ok);
  }
  function errorText(error) {
    return error && error.message ? error.message : String(error);
  }
  function deadline(promise) {
    return new Promise((resolve, reject) => {
      const timer = setTimeout(() => reject(new Error("Timed out after 3 seconds")), 3000);
      Promise.resolve(promise).then(value => {
        clearTimeout(timer); resolve(value);
      }, error => {
        clearTimeout(timer); reject(error);
      });
    });
  }
  async function invoke(method) {
    if (!window.MiniApp || typeof window.MiniApp.invoke !== "function")
      throw new Error("MiniApp SDK unavailable. Open this package in the host application.");
    return deadline(window.MiniApp.invoke(method, {}));
  }
  function checkInfo(info) {
    const keys = ["platform", "appId", "appName", "appVersion", "sdkVersion"];
    if (!info || keys.some(key => typeof info[key] !== "string" || !info[key]))
      throw new Error("The bridge reply is missing expected system information.");
    if (info.appId !== document.body.dataset.appId)
      throw new Error("Bridge returned the wrong appId: " + info.appId);
    return info;
  }
  async function expectError(method, field, expected) {
    try { await invoke(method); }
    catch (error) {
      if (error && error[field] === expected) return "PASS " + method + ": " + expected;
      throw new Error(method + " returned an unexpected error: " + errorText(error));
    }
    throw new Error(method + " unexpectedly succeeded");
  }
  function imageLoads(path) {
    return deadline(new Promise(resolve => {
      const image = new Image();
      image.onload = () => resolve(true);
      image.onerror = () => resolve(false);
      image.src = prefix + path;
    }));
  }
  const actions = {
    format() {
      byId("output").value = JSON.stringify(JSON.parse(byId("input").value), null, 2);
      report("PASS: JSON formatted. Try invalid JSON to check error reporting.");
    },
    minify() {
      byId("output").value = JSON.stringify(JSON.parse(byId("input").value));
      report("PASS: JSON minified.");
    },
    async assets() {
      for (const path of ["icon.svg", "assets/check%20image.svg"])
        if (!await imageLoads(path)) throw new Error("Local image failed to load: " + path);
      if (await imageLoads("assets/missing.svg"))
        throw new Error("A missing local image unexpectedly loaded.");
      report("PASS: SVG assets, encoded spaces and a missing-resource error behaved as expected.");
    },
    async info() { report(checkInfo(await invoke("system.getInfo"))); },
    async parallel() {
      const replies = await Promise.all(Array.from({length: 5}, () => invoke("system.getInfo")));
      replies.forEach(checkInfo);
      report("PASS: all 5 concurrent bridge calls returned the expected appId.");
    },
    async errors() {
      const results = [];
      results.push(await expectError("missing.ping", "code", "unknown_module"));
      results.push(await expectError("badMethod", "code", "invalid_method"));
      results.push(await expectError("system.missing", "message", "unknown action: system.missing"));
      report(results.join("\n"));
    },
    save() {
      localStorage.setItem(noteKey, byId("note").value);
      if (localStorage.getItem(noteKey) !== byId("note").value)
        throw new Error("The saved note did not read back correctly.");
      report("PASS: note saved and read back. Close and reopen to check persistence.");
    },
    restore() {
      const value = localStorage.getItem(noteKey);
      byId("note").value = value === null ? "" : value;
      report(value === null ? "No saved note for this app yet." : "Saved note restored for this app.");
    },
    clear() {
      localStorage.removeItem(noteKey);
      byId("note").value = "";
      report("This app's test note was removed.");
    },
    reload() { location.reload(); }
  };
  document.addEventListener("click", async event => {
    const button = event.target.closest("button[data-action]");
    if (!button) return;
    button.disabled = true;
    try { await actions[button.dataset.action](); }
    catch (error) { report("FAIL: " + errorText(error), false); }
    finally { button.disabled = false; }
  });
  byId("address").textContent = location.href;
  const cssReady = getComputedStyle(document.documentElement).getPropertyValue("--seed-css").trim() === "ready";
  byId("assets-status").textContent = cssReady
    ? "PASS: local JavaScript and CSS loaded." : "FAIL: local CSS did not load.";
  byId("assets-status").dataset.ok = String(cssReady);
  report(byId("assets-status").textContent, cssReady);
  if (byId("note")) {
    try { actions.restore(); }
    catch (error) { report("FAIL: browser storage is unavailable: " + errorText(error), false); }
  }
})();
"""

JSON_BODY = """
<p>Format or minify JSON, try invalid input, use Tab/Shift+Tab, and resize the window.</p>
<label for="input">Input JSON</label>
<textarea id="input" spellcheck="false">{"message":"Hello, Mini App!","items":[1,true,null]}</textarea>
<div class="actions"><button data-action="format">Format</button>
<button data-action="minify">Minify</button><button data-action="assets">Check local assets</button></div>
<label for="output">Output JSON</label><textarea id="output" readonly></textarea>
<p><a href="pages/details.html?case=navigation#details">Open a nested page</a></p>
"""

RUNTIME_BODY = """
<p>These buttons call the actual host SDK. A missing reply fails after 3 seconds.</p>
<div class="actions"><button data-action="info">Get system info</button>
<button data-action="parallel">Run 5 concurrent calls</button>
<button data-action="errors">Check rejected methods</button></div>
<p>System info should identify this app. Error checks must reject an unknown module,
an invalid method name and an unknown system action with the expected replies.</p>
"""

NOTES_BODY = """
<p>Save different text in Notes A and Notes B, then reopen each app. The two apps
deliberately use the same localStorage key to expose storage-isolation problems.</p>
<label for="note">Your test note</label><textarea id="note"></textarea>
<div class="actions"><button data-action="save">Save note</button>
<button data-action="restore">Read saved note</button>
<button data-action="reload">Reload page</button><button data-action="clear">Clear test note</button></div>
<p>Also close and reopen the mini app, then restart the host. Check which operations
retain the note. Browser storage support and lifetime depend on the WebView backend;
an unavailable store is reported as an error, not treated as a successful save.</p>
"""


def make_page(app: tuple, body: str, asset_prefix: str = "") -> str:
    app_id, name, _, _, description = app
    return f"""<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>{html.escape(name)}</title>
<link rel="stylesheet" href="{asset_prefix}assets/style.css">
<script defer src="{asset_prefix}assets/app.js"></script></head>
<body data-app-id="{app_id}" data-asset-prefix="{asset_prefix}">
<header><img src="{asset_prefix}icon.svg" alt=""><div><h1>{html.escape(name)}</h1>
<span>{html.escape(description)}</span></div></header>
<p id="assets-status" role="status">Waiting for local JavaScript and CSS...</p>
<section>{body}</section>
<section><h2>Result</h2><pre id="result" role="status">Waiting for local JavaScript...</pre></section>
<p>Address: <code id="address"></code></p></body></html>
"""


def make_files(app: tuple) -> dict:
    app_id, name, badge, kind, description = app
    manifest = dict(id=app_id, name=name, version="1.0.0", description=description,
                    entry="index.html", icon="icon.svg", permissions=[])
    icon = f"""<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64" viewBox="0 0 64 64">
<rect width="64" height="64" rx="14" fill="#087854"/>
<text x="32" y="41" text-anchor="middle" font-family="sans-serif" font-size="23"
fill="white">{badge}</text></svg>
"""
    body = {"json": JSON_BODY, "runtime": RUNTIME_BODY, "notes": NOTES_BODY}[kind]
    files = {
        "manifest.json": json.dumps(manifest, indent=2) + "\n",
        "index.html": make_page(app, body),
        "icon.svg": icon,
        "assets/style.css": STYLE.strip() + "\n",
        "assets/app.js": JAVASCRIPT.strip() + "\n",
        "assets/check image.svg": icon,
    }
    if kind == "json":
        files["pages/details.html"] = make_page(app, """
<h2 id="details">Nested page</h2>
<p>The query string and fragment should appear in the address below. Local CSS,
JavaScript, images and the SDK should still work after navigation.</p>
<div class="actions"><button data-action="assets">Check local assets</button>
<button data-action="info">Get system info on this page</button></div>
<a href="../index.html">Return to JSON Workbench</a>
""", "../")
    return files


def default_packages_dir(release: bool) -> Path:
    product_info = Path(__file__).resolve().parents[1] / "product/framework/version/product_info.json"
    runtime_id = json.loads(product_info.read_text(encoding="utf-8"))["product"]["runtimeId"]
    folder = f"{runtime_id}App" + ("" if release else "Debug")
    system = platform.system()
    if system == "Windows":
        base = os.environ.get("LOCALAPPDATA") or os.environ.get("USERPROFILE")
        if not base:
            raise RuntimeError("Cannot resolve LOCALAPPDATA / USERPROFILE")
        data_home = Path(base)
    elif system == "Darwin":
        data_home = Path.home() / "Library" / "Application Support"
    elif system == "Linux":
        data_home = Path(os.environ.get("XDG_DATA_HOME") or Path.home() / ".local" / "share")
    else:
        raise RuntimeError(f"Unsupported platform: {system}; use --packages-dir")
    return data_home / folder / "app_data" / "mini_app" / "packages"


def seed_app(packages_dir: Path, app: tuple, dry_run: bool) -> bool:
    destination = packages_dir / app[0]
    if destination.exists() or destination.is_symlink():
        print(f"[skip] {app[1]}: {destination} already exists")
        return False
    if dry_run:
        print(f"[plan] {app[1]} -> {destination}")
        return False
    packages_dir.mkdir(parents=True, exist_ok=True)
    # Publish only a complete package. Temporary parents have no manifest.
    with tempfile.TemporaryDirectory(prefix=".seed-", dir=packages_dir) as temporary:
        staging = Path(temporary) / app[0]
        for relative, content in make_files(app).items():
            target = staging / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(content, encoding="utf-8")
        staging.rename(destination)
    print(f"[installed] {app[1]} -> {destination}")
    return True


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--release", action="store_true", help="Target Release; default is Debug.")
    parser.add_argument("--packages-dir", type=Path, help="Exact packages directory; overrides --release.")
    parser.add_argument("--dry-run", action="store_true", help="Show destinations without writing files.")
    args = parser.parse_args()
    try:
        packages_dir = (args.packages_dir if args.packages_dir is not None
                        else default_packages_dir(args.release)).expanduser().resolve()
        print(f"[info] Packages directory: {packages_dir}")
        installed = sum(seed_app(packages_dir, app, args.dry_run) for app in APPS)
        if args.dry_run:
            print("[done] Dry run; no files were written.")
        else:
            print(f"[done] Installed {installed}; skipped {len(APPS) - installed} existing packages.")
            print("[next] Start/restart the host, open Mini Apps and launch the four Test apps.")
        return 0
    except (OSError, ValueError, KeyError, TypeError, RuntimeError) as error:
        print(f"[error] {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
