Read me for ARToolKit-2.70.1.
=============================


Contents.
---------
About this archive.
Building on Windows.
Building on Linux / SGI Irix.
Building on Mac OS X.
Changes in this release.
Changes in earlier releases.


About this archive.
-------------------
This archive contains the ARToolKit libraries, utilities and examples, version 2.70.1.

ARToolKit is released under the GNU General Public License (GPL). Please read the file COPYING.txt.

The latest version of ARToolKit is available from http://sf.net/projects/artoolkit.

ARToolKit is designed to build on Windows, Linux, SGI Irix, and Macintosh OS X platforms.

This archive was assembled by:
    Philip Lamb
    HIT Lab NZ
    http://www.hitlabnz.org
    2005-03-14


Building on Windows.
--------------------

Prerequisites:
 *  Microsoft Visual Studio .NET 2003 or Visual Studio 6.
 *  DSVideoLib-0.0.4-win32. Download from http://sf.net/projects/artoolkit.
 *  GLUT. Download from http://www.opengl.org/resources/libraries/glut.html.
 *  DirectX 9.0b or later SDK. If you are using VS6, you must use 9.0b as DirectX 9.0c no longer includes support for VS6. Download from http://msdn.microsoft.com/library/default.asp?url=/downloads/list/directx.asp.
 *  (DirectX 9.0c October 2004 or later only) DirectX SDK Extras package. Once downloaded and unzipped, move the "Samples" folder into the top-level of the installed SDK path.
 *  (Optional, for VRML renderer only) OpenVRML-0.14.3-win32. Download from http://sf.net/projects/artoolkit.
 
Build steps:
(1) Unpack the ARToolKit zip to a convenient location. This location will be referred to below as {ARToolKit}.
(2) Unpack the DSVideoLib zip into {ARToolKit}.
(3) Copy the files DSVideoLib.dll and DSVideoLibd.dll from {ARToolKit}\DSVideoLib\bin.vc70 into {ARToolKit}\bin.
(4) Run the script {ARToolKit}\DSVideoLib\bin.vc70\register_filter.bat.
(5) Install the GLUT DLL into the Windows System32 folder, and the library and headers into the VS platform SDK folders.
(6) Run the script {ARToolKit}\Configure.win32.bat to create include/AR/config.h.
(7) Open the ARToolKit.sln file (VS.NET) or ARToolkit.dsw file (VS6).
(8) Open the Visual Studio search paths settings (Tools->Options->Directories for VS6, or Tools->Options->Projects->VC++ Directories for VS.NET) and add the DirectX SDK Includes\ path and the DirectX Samples\C++\DirectShow\BaseClasses\ path to the top of the search path for headers, and the DirectX SDK Lib\ path to the top of the search path for libraries.
(9) (Optional, only if rebuilding DSVideoLib). Build the DirectShow base classes strmbase.lib and strmbasd.lib. (More information can be found at Thomas Pintarics homepage for DSVideoLib (http://www.ims.tuwien.ac.at/~thomas/dsvideolib.php)).
(10) Build the toolkit.

The VRML renderering library and example (libARvrml & simpleVRML) are optional builds:
(11) Unpack the OpenVRML zip into {ARToolKit}.
(12) Copy js32.dll from {ARToolKit}\OpenVRML\bin into {ARToolKit}\bin.
(13) Enable the libARvrml and simpleVRML projects in the VS configuration manager and build.


Building on Linux / SGI Irix.
-----------------------------

Prerequisites:
 *  (Optional, for VRML renderer only) openvrml-0.14.3 and dependencies. Download from http://sf.net/projects/openvrml.
 
Unpack the ARToolKit to a convenient location. The root of this location will be referred to below as {ARToolKit}:
    tar zxvf ARToolKit-2.70.1.tgz
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
 *  (Optional, for VRML renderer only) Apple X11 and Fink. Download X11 from http://www.apple.com/downloads/macosx/apple/x11formacosx.html. Download Fink (and optionally, FinkCommander) from http://fink.sf.net.

Building using the XCode IDE:
(1) Unpack the archive to a convenient location using StuffIt Expander, and open the ARToolKit.xproj.
(2) Builds include a script target "Configure" which enables accelerated and rectangular texturing by default. If you wish to change these defaults, manually run the ./Configure script from Terminal as for a command-line build.
(3) Executables are built into ARToolKit/bin. This must also be the working directory when the executables are run, but by default XCode runs executables with the working directory set to Build/. In order for the binaries to be able to be run from within XCode you will need to edit the "Working directory path" of each executable's XCode settings to the full path to ARToolKit/bin on your system.

The VRML renderering library and example (libARvrml & simpleVRML) are optional builds:
(4) Using FinkCommander, do a binary install of mozilla-dev, followed by an install of openvrml4-dev and openvrml-gl5-dev.
(5) Select the ARToolKit extensions target, and build.

Alternately, ARToolKit can be built from the Terminal, using the Unix makefiles.
Drop the ARToolKit into a convenient location, e.g. your Desktop, then open a Terminal window and type:
	cd ~/Desktop
	tar zxvf ARToolKit-2.70.1.tgz
Configure and build
	cd ~/ARToolKit
	./Configure
	make
Following a successful build, to run a binary such as simpleTest, add these commands:
	cd bin
	./simpleTest

The VRML renderering library and example (libARvrml & simpleVRML) are optional builds:
	fink -b install mozilla-dev
	fink install openvrml4-dev openvrml-gl5-dev
	cd ~/Desktop/ARToolKit/lib/SRC/ARvrml
	make
	cd ~/Desktop/ARToolKit/examples/simpleVRML
	make
	cd ~/Desktop/ARToolKit/bin
	./simpleVRML


Changes in this release.
------------------------
- Mac OS X video driver: support added for runtime specification of pixel format.
- Mac OS X video driver: video input settings are now saved to and restored from preferences.
- Linux 1394cam video driver: Patch by Henrik Erkkonen to support version 11 of libdc1394.

Changes in earlier releases.
----------------------------
Please see the file ChangeLog.txt.

--
EOF