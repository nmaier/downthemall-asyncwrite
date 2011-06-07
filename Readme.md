DownThemAll! Async Write
========================
aka. DownThemAll! Background File Write Windows Extension


Purpose
---
DownThemAll! file writes are currently somewhat inefficient due to several issues and constraints

* All open streams write basically concurrently
* Write occur on the main thread (not on a background thread)
* The underlying mixture of nsIFileStreams and NSPR is not optimized

Fixing this in JS in not entirely feasible:

* Cannot use nsIBufferedStreams any longer, as the underlying custom-nsIFileStream implementation in js truncates the buffer (the interfaces specifies stream, which XPConnect interprets as a null-terminated stream)
* Any additional layer in js has horrible performance characteristics, mainly because all the XPConnect hoping, incl. data copies it would result in

This extension hence explores the use of a custom nsIFileOutputStream implementation, that will write data only using a dedicated (background) thread.
As a side-effect, this fixes the pre-allocation issue, as pre-alloc are not absolutely necessary but still can be performed, but this time in a background thread.


Status
---
The current code is currently in an early beta state and will only work against Firefox 4 and 5/DownThemAll! 2.1 alpha (trunk).

It should be considered an experiment, that might later be abandoned again (if background thread I/O in Firefox Chromeworkers eventually arrives), or might be polished and make it into the final product, although we so far tried to be a JS-only extension. JS-only fallbacks are possible, so it's not that much of an issue.
The code now supports Win and *nix incl. Darwin. Other platforms likely only have to implement (or omit) `pr.c:file_seteof()`.

Currently the following binaries are pre-built and within the repository:

* Windows 32 (MSVC9+PGO)
* Linux x86 (gcc-4.4)
* Linux x86_64 (gcc-4.4)
* Darwin x86 (gcc-4.0.1; Leopard)

Code
---
The code is organized as a XPCOM component in C++, and some "low-level" libraries (delayed_stream, pr).
This is mostly due to historic reasons, as the first attempt was to implement this thing as a low-level library to be used from js-ctypes.
However, it turns out that js-ctypes has far too much (memory) overhead.

Known issues and limitations
---

* Firefox 4 or 5/mozilla-2.0 is required; a compat layer to build mozilla-1.9.x components is not present yet, and likely never will. Added to this, the shipped binary component links against the custom CRT, as shipped by Firefox 4. Firefox 6 would require rebuilding the components as the XPCOM module version changed.
* The code might still lock the UI, but this should be only noticable when a stream is closed again. The application then has to wait for the data to be written and the underlying file handles to be closed, as there is no async close support in dTa (yet)


Reusing
---
If you'd like to reuse the component in your own extension, the please change the interface name, UUID and contract id of the implementation!
Else a user might install conflicting versions, where the either your or our extension doesn't work anymore.
Hence, please DO NOT ship the binaries from our XPIs or the repository for that very same reason.
Do NOT ship the pre-build binaries, or I'll have to hunt you down and hit you repeatedly :p (Ask FireFTP/FireGPG and other IPC library consumers why).
Neither the interfaces nor the implementation is frozen. Both is subject to massive, unannounced changes at this point.


Building on Windows
---

The major part of this extension is the AsyncFileOutputStream XPCOM component and support libraries.
A Visual Studio project is provided, but you need to adjust the path to the xulrunner-sdk-2.0/-5.0 files. The typelib and header from the interface (idl) need to be built outside of VisualStudio:

* `AFOS # $XULSDK/bin/xpidl -I $XULSDK/idl -m typelib dtaIAsyncFileOutputStream.idl`
* `AFOS # mv dtaIAsyncFileOutputStream.xpt ../`
* `AFOS # $XULSDK/bin/xpidl -I $XULSDK/idl -m header dtaIAsyncFileOutputStream.idl`

Also, if you'd like to link against the mozcrt19, then you need a working import library. Otherwise, remove the ignored msvcrt.lib from the linker flags again and use the static (not DLL) crt.

Building on *nix
---

While SDK builds were an option, it is usually safer to build within a mozilla tree. Hence only this build variant is supported.

* Clone mozilla-2.0 or mozilla-beta (depending on Firefox 5 support)
* Clone this repo under extensions
* Add `ac_add_options --enable-extensions=default,downthemall-asyncwrite`
* Build
* The component will be staged into `$objdir/dist/xpi-stage/AsyncFileOutputStream/components/libAsyncFileOutputStream-$(uname -p).{so,dylib}`

License
---
All files are either licensed under the MPL, or dedicated to the public domain. See the license block in the corresponding files!
