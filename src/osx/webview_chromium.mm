///////////////////////////////////////////////////////////////////////////////
// Name:        src/osx/webview_chromium.mm
// Purpose:     OSX specific code for wxWebViewChromium
// Author:      Tobias Taschner
// Created:     2018-02-19
// Copyright:   (c) 2018 wxWidgets development team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "wx/webview_chromium.h"

#import "Cocoa/Cocoa.h"
#include <objc/runtime.h>
#include "wx/log.h"
#import "include/cef_application_mac.h"

/*
   The objective c runtime is used to add the required CefAppProtoco to the
     wxNSApplication.sharedApplication

   This protocol is required by chromium in different places
   (e.g. <select> drop downs, right click)
*/

@interface wxCEFNSApplication : NSObject<CefAppProtocol>
{
@private
    BOOL handlingSendEvent_;
}

@end

@implementation wxCEFNSApplication

- (BOOL)isHandlingSendEvent {
    return handlingSendEvent_;
}

- (void)setHandlingSendEvent:(BOOL)handlingSendEvent {
    handlingSendEvent_ = handlingSendEvent;
}

@end

id isHandlingSendEventImpl(id self, SEL, ...)
{

    return nil;
}

id setHandlingSendEventImpl(id self, SEL, ...)
{

    return nil;
}

void wxWebViewChromium_InitOSX()
{
    Protocol* prot = objc_getProtocol("CefAppProtocol");
    if (prot)
    {
		Class appClass = objc_getClass("wxNSApplication");
        class_addProtocol(appClass, prot);
        if (!class_addMethod(appClass, @selector(isHandlingSendEvent), &isHandlingSendEventImpl, "c@:"))
            wxLogError("Could not add isHandlingSendEvent impl");
        if (!class_addMethod(appClass, @selector(setHandlingSendEvent:), &isHandlingSendEventImpl, "v@:c"))
            wxLogError("Could not add setHandlingSendEvent impl");
    }
}
