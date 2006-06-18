Read me for ARToolKit-2.71.3.
=============================


Contents.
---------
About this archive.
Building on Windows.
Building on Linux / SGI Irix.
Building on Mac OS X.
Changes in this release.
Known issues in this release.
Changes in earlier releases.


About this archive.
-------------------
This archive contains the ARToolKit libraries, utilities and examples, version 2.71.3.

ARToolKit is released under the GNU General Public License (GPL). Please read the file COPYING.txt.

The latest version of ARToolKit is available from http://sf.net/projects/artoolkit.

ARToolKit is designed to build on Linux, Windows, Macintosh OS X, and SGI Irix platforms.

This archive was assembled by:
    Philip Lamb
    HIT Lab NZ
    http://www.hitlabnz.org
    2006-06-19


Building on Windows.
--------------------

Prerequisites:
 *  Microsoft Visual Studio .NET 2003 or Visual Studio 6, or a free development environment such as Cygwin.
 *  DSVideoLib-0.0.8b-win32. Download from http://sf.net/projects/artoolkit.
 *  GLUT. Download from http://www.xmission.com/~nate/glut/glut-3.7.6-bin.zip.
 *  (Optional, for VRML renderer only) OpenVRML-0.14.3-win32. Download from http://sf.net/projects/artoolkit.
 
Build steps:
(1) Unpack the ARToolKit zip to a convenient location. This location will be referred to below as {ARToolKit}.
(2) Unpack the DSVideoLib zip into {ARToolKit}. Make sure that the directory is named "DSVL".
(3) Copy the files DSVL.dll and DSVLd.dll from {ARToolKit}\DSVL\bin into {ARToolKit}\bin.
(4) Install GLUT, following the instructions in the README.win file inside the GLUT zip. See http://www.hitlabnz.org/forum/showpost.php?p=332&postcount=12 for more detail on how to install GLUT on Windows.
(5) Run the script {ARToolKit}\Configure.win32.bat to create include\AR\config.h.
(6) Open the ARToolKit.sln file (VS.NET) or ARToolkit.dsw file (VS6).
(7) Build the toolkit.

The VRML renderering library and example (libARvrml & simpleVRML) are optional builds:
(8) Unpack the OpenVRML zip into {ARToolKit}.
(9) Copy js32.dll from {ARToolKit}\OpenVRML\bin into {ARToolKit}\bin.
(10) Enable the libARvrml and simpleVRML projects in the VS configuration manager and build.


Building on Linux / SGI Irix.
-----------------------------

Prerequisites:
 *  (Optional, for VRML renderer only) openvrml-0.14.3 and dependencies. Download from http://sf.net/projects/openvrml.
 
Unpack the ARToolKit to a convenient location. The root of this location will be referred to below as {ARToolKit}:
    tar zxvf ARToolKit-2.71.3.tgz
Configure and build. The Linux builds support video input using either Video4Linux, an IIDC-compliant or DV camera connected via IEEE-1394, or a Sony EyeToy camera connected via USB. You will be prompted as to which of the four Linux video drivers you wish to use at the Configure step.
    cd {ARToolKit}
    ./Configure
	make
Following a successful build, to run a binary such as simpleTest:
	cd {ARToolKit}/bin
	./simpleTest

The VRML renderering library and example (libARvrml & simpleVRML) are optional builds:
	cd {ARToolKit}/lib/SRC/ARvrml
	make
	cd {ARToolKit}/examples/simpleVRML
	make
	cd {ARToolKit}/bin
	./simpleVRML


Building on Mac OS X.
---------------------------

Prerequisites:
 *  Apple's Developer Tools. http://developer.apple.com
 *  Camera driver. Mac OS X 10.3 and later include an inbuilt IIDC and DV FireWire camera driver. USB camera drivers, and Firewire camera drivers for pre-10.3 systems can be obtained from ioXperts at http://www.ioxperts.com.
 *  (Optional, for VRML renderer only) Apple X11. Install X11 from the Mac OS X installer, from the "Optional installs" package,  or download an install disk image from http://www.apple.com/downloads/macosx/apple/x11formacosx.html.
 *  (Optional, for VRML renderer only) OpenVRML-0.14.3-MacOSX.tar.gz. Download from http://sf.net/projects/artoolkit, or alternatively install via Fink. Download Fink (and optionally, FinkCommander) from http://fink.sourceforge.net/download/.

Building using the XCode IDE:
(1) Unpack the archive to a convenient location using StuffIt Expander, and open the ARToolKit.xproj.
(2) Builds include a script target "Configure" which enables accelerated and rectangular texturing by default. If you wish to change these defaults, manually run the ./Configure script from Terminal as for a command-line build.
(3) Mac OS X XCode builds now build the examples as bundled applications. The contents of the "Data" directory are copied into the application bundle at build time. The applications can thus be moved from their build locations. The utilities are still (mostly) built as command-line tools.

The VRML renderering library and example (libARvrml & simpleVRML) are optional builds:
(4) Unpack the OpenVRML .tar.gz into the ARToolKit folder, or alternatively, if using FinkCommander, do a binary install of mozilla-dev, followed by an install of openvrml4-dev and openvrml-gl5-dev.
(5) Select the ARToolKit extensions target, and build.

Alternately, ARToolKit can be built from the Terminal, using the Unix makefiles.
Drop the ARToolKit into a convenient location, e.g. your Desktop, then open a Terminal window and type:
	cd ~/Desktop
	tar zxvf ARToolKit-2.71.3.tgz
Configure and build
	cd ~/ARToolKit
	./Configure
	make
Following a successful build, to run a binary such as simpleTest, add these commands:
	cd bin
	./simpleTest

The VRML renderering library and example (libARvrml & simpleVRML) are optional builds:
	Either:
		tar xzvf OpenVRML-0.14.3-MacOSX.tar.gz ~/Desktop/ARToolKit
	Or:
		fink -b install mozilla-dev
		fink install openvrml4-dev openvrml-gl5-dev
	Then:
	cd ~/Desktop/ARToolKit/lib/SRC/ARvrml
	make
	cd ~/Desktop/ARToolKit/examples/simpleVRML
	make
	cd ~/Desktop/ARToolKit/bin
	./simpleVRML


Changes in version 2.71.3 (this release) (2006-06-19).
------------------------------------------------------
- This release supports Mac OS X on Intel, and builds Universal binaries, with the exception of the VRML projects (since OpenVRML does not yet build on Mac OS X i386).
- Fixed line endings in MS Visual Studio 6 project files calib_distortion.dsp and calib_camera2.dsp.
- Fix for a bug in VideoLinuxDV identified by Simon Goodall, where video stream could be incorrectly closed twice (SF 1452403).
- The Win32 video library now accepts in the config string either the pathname of an XML file, or raw XML. The string is treated as raw XML if the  first 5 characters are "<?xml".
- The Mac OS X video library won't return a copy of the acquired image if running on an Intel Mac. This is a workaround for a driver issue. It may cause problems and a more robust solution is coming.
- Fixed a bug in pattern extraction which occured when AR_PATTERN_SIZE_X and AR_PATTERN_SIZE_Y were not equal.
- Rewrite of mk_patt.c to use gsub_lite.
- Cleaned up pixel format specification in config.h, borrowing code from gsub_lite.
- Fix long-standing bug in gsub_lite when drawing to fullscreen windows in drawpixels mode.


Known issues in this release.
-----------------------------
- On Mac OS X, the macam webcam driver may cause problems such as the camera settings dialog exploding and hanging the app. A workaround is to either remove the file /Library/Quicktime/macam.component, or to add "-standarddialog" to the video configuration string.
- On Windows, MSVS.Net 2005 is not yet supported. The toolkit will build under it, with the exception of libARvrml and simpleVRML.


Changes in version 2.71.2 (2005-10-11).
-------------------------------------
- Fixed line endings in MS Visual Studio 6 workspace (.dsw) file.


Changes in version 2.71.1 (2005-09-22).
---------------------------------------
- Mac OS X XCode builds: An outdated version of the project file slipped into the 2.71 release. This has been corrected.
- Mac OS X XCode builds: Version 2.1 of XCode is required. The obsolete XCode 2.0 .xcode project file has been removed. Users running on Mac OS X versions prior to 10.4 can still build using the command-line Makefiles.
- A bug in libARgsub_lite incorrectly restored OpenGL state. Fixed. Also corrected for this fix in simpleLite and twoView examples.
- Builds of calib_cparam now correctly link against libARgsub_lite.


Changes in version 2.71 (2005-09-21).
-------------------------------------
- Mac OS X video driver: QuickTime 6.4 is now required by default. (Support for  versions back to 4.0 can be enabled at compile-time if required). 
- Mac OS X libARgsub and binaries which call it: fix for bug "GLUT Warning: glutInit being called a second time" by moving glutInit to main so that it is called before calling arVideoOpen.
- Linux V4L video driver: Apply patch by Wayne Piekarski to auto-adjust video resolution.
- Windows video driver: Uses updated (0.0.8b) DSVideoLib. Building ARToolKit on Windows no longer requires installation of DirectX SDK, or registering of DS filters. It also includes many bug fixes.
- gsub_lite: Added complete support for runtime specification of pixel format and rectangle/power-of-2 textures.
- Mac OS X XCode builds now build the examples as bundled applications. The contents of the "Data" directory are copied into the application bundle at build time. The applications can thus be moved from their build locations. The utilities are still (mostly) built as command-line tools.
- The calibration utilties have been rewritten to use gsub_lite. They should now run much more reliably across different platforms.


Changes in earlier releases.
----------------------------
Please see the file ChangeLog.txt.

--
EOF