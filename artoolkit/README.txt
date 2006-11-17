Read me for ARToolKit-2.72.
===========================


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
This archive contains the ARToolKit libraries, utilities and examples, version 2.72.

ARToolKit is released under the GNU General Public License (GPL). Please read the file COPYING.txt.

The latest version of ARToolKit is available from http://sf.net/projects/artoolkit.

ARToolKit is designed to build on Linux, Windows, Macintosh OS X, and SGI Irix platforms.

This archive was assembled by:
    Philip Lamb
    HIT Lab NZ
    http://www.hitlabnz.org
    2006-11-20


Building on Windows.
--------------------

Prerequisites:
 *  Microsoft Visual Studio .NET 2003 or Visual Studio 6, or a free development environment such as Cygwin.
 *  DSVideoLib-0.0.8b-win32. Download from http://sf.net/projects/artoolkit.
 *  GLUT. Download from http://www.xmission.com/~nate/glut/glut-3.7.6-bin.zip.
 *  (Optional, for VRML renderer only) OpenVRML-0.16.1-bin-win32. Download from http://sf.net/projects/artoolkit.
 
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
 *  (Optional, for VRML renderer only) openvrml-0.16.1 and dependencies. Download from http://sf.net/projects/openvrml.
 
Unpack the ARToolKit to a convenient location. The root of this location will be referred to below as {ARToolKit}:
    tar zxvf ARToolKit-2.72.tgz
Configure and build. The Linux builds support video input using either Video4Linux, an IIDC-compliant or DV camera connected via IEEE-1394, or a Sony EyeToy camera connected via USB. Alternatively you can use GStreamer 0.10 (0.8 is not supported and also not recommended) as input method. This requires you to install the gstreamer development packages for your Linux distribution. You will be prompted as to which of the four Linux video drivers you wish to use at the Configure step.
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
 *  Camera driver. Mac OS X 10.3 and later include an inbuilt IIDC and DV FireWire camera driver. The macam project provides USB camera drivers for OS X, http://webcam-osx.sf.net.
 *  (Optional, for VRML renderer only) Apple X11. Install X11 from the Mac OS X installer, from the "Optional installs" package,  or download an install disk image from http://www.apple.com/downloads/macosx/apple/x11formacosx.html.
 *  (Optional, for VRML renderer only) OpenVRML-0.16.16-bin-MacOSX.tar.gz. Download from http://sf.net/projects/artoolkit, or alternatively install via Fink. Download Fink (and optionally, FinkCommander) from http://fink.sourceforge.net/download/.

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
	tar zxvf ARToolKit-2.72.tgz
Configure and build
	cd ~/ARToolKit
	./Configure
	make
Following a successful build, to run a binary such as simpleTest, add these commands:
	cd bin
	./simpleTest

The VRML renderering library and example (libARvrml & simpleVRML) are optional builds:
	Either:
		tar xzvf OpenVRML-0.16.16-bin-MacOSX.tar.gz ~/Desktop/ARToolKit
	Or:
		fink -b install mozilla-dev
		fink install openvrml6-dev openvrml-gl6-dev
	Then:
	cd ~/Desktop/ARToolKit/lib/SRC/ARvrml
	make
	cd ~/Desktop/ARToolKit/examples/simpleVRML
	make
	cd ~/Desktop/ARToolKit/bin
	./simpleVRML


Changes in version 2.72 (this release) (2006-11-20).
------------------------------------------------------
All platforms:
New feature: ARToolKit now supports querying of SDK and runtime versions.
New feature: There are now right-hand coordinate system versions of the OpenGL projection and modelview matrix functions, named arglCameraFrustumRH and arglCameraViewRH, respectively. These will help when integrating ARToolKit into existing OpenGL drawing.
Enhancement: Support for AR_PIXEL_FORMAT_MONO is now included.
Enhancement: gsub_lite now supports arDebug mode.
Enhancement: ARvrml now builds against OpenVRML-0.16.1.

Mac OS X:
Bug fix: Fix for an error in the VRML library inclusion on Mac OS X.
Bug fix: Now detects if running emulated on Intel Macs, and optimises video for this case.
Enhancement: Two new video config tokens are now available; -fliph and -flipv, which will mirror the video image horizontally and vertically respectively.

Linux:
New feature: Support for gstreamer video capture added, thanks to Hartmut Seichter.
Enhancement: Major changes to Video1394DC- add PointGray DragonFly support, add LGPL/GPL license info, better config string support, stabilised interfaces, lots of clean ups to make the code easier to work with.
Bug fix: Using PAL in VideoLinuxDV as reported at http://www.hitlabnz.org/forum/showthread.php?t=412.


Changes in earlier releases.
----------------------------
Please see the file ChangeLog.txt.

--
EOF
