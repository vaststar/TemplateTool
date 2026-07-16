#pragma once

#ifdef __linux__

#include <ucf/Utilities/ScreenCaptureUtils/ScreenCaptureUtils.h>

namespace ucf::utilities::screencapture {

/// Result of a single portal ScreenCast frame grab.
///
/// The image is the full virtual desktop as delivered by one PipeWire stream.
/// @c regionX / @c regionY give the top-left position of this stream inside the
/// virtual desktop (as reported by the portal), so callers can crop a specific
/// monitor without taking another screenshot.
struct PortalFrame
{
    CaptureImage image;   ///< Raw BGRA pixels (empty on failure)
    int regionX = 0;      ///< Stream origin X in the virtual desktop
    int regionY = 0;      ///< Stream origin Y in the virtual desktop
};

/// Grab a single full-desktop frame via xdg-desktop-portal ScreenCast + PipeWire.
///
/// Performs the CreateSession -> SelectSources -> Start -> OpenPipeWireRemote
/// negotiation over D-Bus, connects to the returned PipeWire node, copies the
/// first frame into a BGRA CaptureImage, then tears everything down.
///
/// No temp files, no PNG, no external processes. Returns an invalid image on
/// failure (portal denied, no Wayland session, PipeWire unavailable, etc.).
PortalFrame grabPortalFrameBGRA();

/// True if a portal restore token has been stored from a previous authorization,
/// meaning a subsequent grab should proceed without a permission dialog.
bool portalHasStoredToken();

} // namespace ucf::utilities::screencapture

#endif // __linux__
