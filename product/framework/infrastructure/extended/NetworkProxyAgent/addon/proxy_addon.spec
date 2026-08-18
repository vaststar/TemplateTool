# -*- mode: python ; coding: utf-8 -*-

from PyInstaller.utils.hooks import collect_all


mitmproxy_datas, mitmproxy_binaries, mitmproxy_hiddenimports = collect_all(
    'mitmproxy'
)
rs_datas, rs_binaries, rs_hiddenimports = collect_all('mitmproxy_rs')


a = Analysis(
    ['proxy_addon.py'],
    pathex=[],
    binaries=mitmproxy_binaries + rs_binaries,
    datas=mitmproxy_datas + rs_datas,
    hiddenimports=(
        mitmproxy_hiddenimports
        + rs_hiddenimports
        + [
            'mitmproxy',
            'mitmproxy.addons',
            'mitmproxy.tools',
            'mitmproxy.tools.main',
        ]
    ),
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name='proxy_addon',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
coll = COLLECT(
    exe,
    a.binaries,
    a.datas,
    strip=False,
    upx=True,
    upx_exclude=[],
    name='proxy_addon',
)
