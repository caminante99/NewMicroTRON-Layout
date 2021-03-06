//
//  ThreeFingerRelativeToScrollAppDelegate.m
//  ThreeFingerRelativeToScroll
//
//  Created by Takayama Fumihiko on 09/11/24.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "ThreeFingerRelativeToScrollAppDelegate.h"

@implementation ThreeFingerRelativeToScrollAppDelegate

- (id) init
{
  self = [super init];

  if (self) {
    mtdevices_ = [NSMutableArray new];
  }

  return self;
}

- (void) dealloc
{
  [mtdevices_ release];

  [super dealloc];
}

// ------------------------------------------------------------
struct Finger;
typedef void* MTDeviceRef;
typedef int (* MTContactCallbackFunction)(int, struct Finger*, int, double, int);

CFMutableArrayRef MTDeviceCreateList(void);
void MTRegisterContactFrameCallback(MTDeviceRef, MTContactCallbackFunction);
void MTUnregisterContactFrameCallback(MTDeviceRef, MTContactCallbackFunction);
void MTDeviceStart(MTDeviceRef, int);
void MTDeviceStop(MTDeviceRef, int);

org_pqrs_KeyRemap4MacBook_Client* global_client_ = nil;
NSMutableArray* global_mtdevices_ = nil;

static void setPreference(int newvalue) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [[global_client_ proxy] setValueForName:newvalue forName:@"notsave.pointing_relative_to_scroll"];
  [pool drain];
}

// ------------------------------------------------------------
// Multitouch callback
static int callback(int device, struct Finger* data, int fingers, double timestamp, int frame) {
  static int current = 0;
  int newstatus = (fingers >= 3 ? 1 : 0);

  if (current != newstatus) {
    current = newstatus;
    setPreference(current);
  }

  return 0;
}

static void setcallback(BOOL isset) {
  NSArray* list = nil;
  NSEnumerator* e = nil;

  list = (NSArray*)(MTDeviceCreateList());
  if (! list) goto finish;

  e = [list objectEnumerator];
  if (! e) goto finish;

  for (;;) {
    MTDeviceRef device = [e nextObject];
    if (! device) break;

    // We need retain 'device' to prevent a mysterious crash.
    // So, we append 'device' to global_mtdevices_.
    if ([global_mtdevices_ indexOfObject:device] == NSNotFound) {
      [global_mtdevices_ addObject:device];
    }

    if (isset) {
      MTRegisterContactFrameCallback(device, callback);
      MTDeviceStart(device, 0);
    } else {
      MTUnregisterContactFrameCallback(device, callback);
      MTDeviceStop(device, 0);
    }
  }

finish:
  [list release];
}

// ------------------------------------------------------------
// Notification
static void observer_refresh(void* refcon, io_iterator_t iterator) {
  NSLog(@"[INFO] observer_refresh called\n");

  setPreference(0);

  bool isfound = false;
  while (IOIteratorNext(iterator)) {
    isfound = true;
  }
  if (isfound) {
    // wait for the initialization of the device
    sleep(1);

    setcallback(NO);
    setcallback(YES);
  }
}

- (void) setNotification {
  IONotificationPortRef port = IONotificationPortCreate(kIOMasterPortDefault);
  if (! port) {
    NSLog(@"[ERROR] IONotificationPortCreate");
    return;
  }

  // ------------------------------------------------------------
  NSMutableDictionary* subdict = [NSMutableDictionary dictionaryWithObject:@"AppleMultitouchHIDEventDriver" forKey:@"IOClass"];
  NSMutableDictionary* match = [NSMutableDictionary dictionaryWithObject:subdict forKey:@kIOPropertyMatchKey];

  // ----------------------------------------------------------------------
  io_iterator_t it;
  kern_return_t kr;

  [match retain]; // for kIOTerminatedNotification
  kr = IOServiceAddMatchingNotification(port,
                                        kIOTerminatedNotification,
                                        (CFMutableDictionaryRef)match,
                                        &observer_refresh,
                                        NULL,
                                        &it);
  if (kr != kIOReturnSuccess) {
    NSLog(@"[ERROR] IOServiceAddMatchingNotification");
    return;
  }
  observer_refresh(NULL, it);

  [match retain]; // for kIOMatchedNotification
  kr = IOServiceAddMatchingNotification(port,
                                        kIOMatchedNotification,
                                        (CFMutableDictionaryRef)match,
                                        &observer_refresh,
                                        NULL,
                                        &it);
  if (kr != kIOReturnSuccess) {
    NSLog(@"[ERROR] IOServiceAddMatchingNotification");
    return;
  }
  observer_refresh(NULL, it);

  // ----------------------------------------------------------------------
  CFRunLoopSourceRef loopsource = IONotificationPortGetRunLoopSource(port);
  if (! loopsource) {
    NSLog(@"[ERROR] IONotificationPortGetRunLoopSource");
    return;
  }
  CFRunLoopAddSource(CFRunLoopGetCurrent(), loopsource, kCFRunLoopDefaultMode);
}

// ------------------------------------------------------------
- (void) applicationDidFinishLaunching:(NSNotification*)aNotification {
  global_client_ = client_;
  global_mtdevices_ = mtdevices_;
  [self setNotification];

  // --------------------------------------------------
  NSStatusBar* statusBar = [NSStatusBar systemStatusBar];
  statusItem_ = [statusBar statusItemWithLength:24];
  [statusItem_ retain];

  [statusItem_ setTitle:@""];
  [statusItem_ setImage:[NSImage imageNamed:@"icon.statusbar.0"]];
  [statusItem_ setAlternateImage:[NSImage imageNamed:@"icon.statusbar.1"]];
  [statusItem_ setHighlightMode:YES];
  [statusItem_ setMenu:statusMenu_];
}

- (void) applicationWillTerminate:(NSNotification*)aNotification {
  setcallback(NO);

  setPreference(0);

  [statusItem_ release];
}

// ------------------------------------------------------------
- (IBAction) quit:(id)sender {
  [NSApp terminate:self];
}

// ------------------------------------------------------------
- (NSURL*) appURL {
  return [NSURL fileURLWithPath:[[NSBundle mainBundle] bundlePath]];
}

- (LSSharedFileListItemRef) getLSSharedFileListItemRef:(LSSharedFileListRef)loginItems {
  if (! loginItems) return NULL;

  LSSharedFileListItemRef retval = NULL;
  NSURL* appURL = [self appURL];

  UInt32 seed = 0U;
  NSArray* currentLoginItems = [NSMakeCollectable (LSSharedFileListCopySnapshot(loginItems, &seed))autorelease];
  for (id itemObject in currentLoginItems) {
    LSSharedFileListItemRef item = (LSSharedFileListItemRef)itemObject;

    UInt32 resolutionFlags = kLSSharedFileListNoUserInteraction | kLSSharedFileListDoNotMountVolumes;
    CFURLRef url = NULL;
    OSStatus err = LSSharedFileListItemResolve(item, resolutionFlags, &url, NULL);
    if (err == noErr) {
      BOOL foundIt = CFEqual(url, appURL);
      CFRelease(url);

      if (foundIt) {
        retval = item;
        break;
      }
    }
  }

  return retval;
}

- (BOOL) isStartAtLogin {
  LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
  if (! loginItems) return NO;

  LSSharedFileListItemRef item = [self getLSSharedFileListItemRef:loginItems];
  CFRelease(loginItems);

  return item != NULL;
}

- (void) enableStartAtLogin {
  LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
  if (! loginItems) return;

  NSURL* appURL = [self appURL];
  LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(loginItems, kLSSharedFileListItemLast, NULL, NULL, (CFURLRef)(appURL), NULL, NULL);
  if (item) {
    CFRelease(item);
  }
  CFRelease(loginItems);
}

- (void) disableStartAtLogin {
  LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
  if (! loginItems) return;

  LSSharedFileListItemRef item = [self getLSSharedFileListItemRef:loginItems];
  if (item) {
    LSSharedFileListItemRemove(loginItems, item);
  }
  CFRelease(loginItems);
}

// ------------------------------------------------------------
- (IBAction) setStartAtLogin:(id)sender {
  if ([self isStartAtLogin]) {
    [self disableStartAtLogin];
    [sender setState:NSOffState];
  } else {
    [self enableStartAtLogin];
    [sender setState:NSOnState];
  }
}

- (void) menuNeedsUpdate:(NSMenu*)menu {
  if ([self isStartAtLogin]) {
    [startAtLoginMenuItem_ setState:NSOnState];
  } else {
    [startAtLoginMenuItem_ setState:NSOffState];
  }
}

@end
