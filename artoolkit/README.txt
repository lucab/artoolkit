Read me for ARToolKit-2.69.
===========================


Contents.
---------
About this archive.
Building on Windows.
Building on Linux / SGI Irix.
Building on Mac OS X.
Latest changes.
Previous changes.


About this archive.
-------------------
This archive contains the ARToolKit libraries, utilities and examples, version 2.69.

ARToolKit is released under the GNU General Public License (GPL). Please read the file COPYING.txt.

ARToolKit is designed to build on Windows, Linux, SGI Irix, and Macintosh OS X platforms, and is available in either a compressed archive (.tar.gz) source-only distribution, or as a compiled binary distribution for the individual supported platforms.

As at 2004-11-04, this package does not include support for the ARToolKit VRML renderer. Support will be forthcoming as a separate package at a later date.

This archive was assembled by:
	Philip Lamb
	HIT Lab NZ
	http://www.hitlabnz.org
	2004-11-04


Building on Windows.
--------------------

Building ARToolKit from source on Windows proceeds by the following steps:
(1) Unpack the ARToolKit zip file to a convenient location. This location will be referred to below as {ARToolKit}.
(2) (Source-only distributions only). Download the DSVideoLib0.0.4.zip package and unpack it into {ARToolKit}.
(3) (Source-only distributions only). Open the folder {ARToolKit}\DSVideoLib\bin.vc60 (Visual Studio 6.0) or {ARToolKit}\DSVideoLib\bin.vc70 (Visual Studio .NET 2003) and copy the files DSVideoLib.dll and DSVideoLibd.dll into {ARToolKit}\bin.
(4) Open the folder {ARToolKit}\DSVideoLib\bin.vc60 (VS6) or {ARToolKit}\DSVideoLib\bin.vc70 (VS.NET) and run the script register_filter.bat.
(5) Install the GLUT DLL into the Windows System32 folder, and the library and headers into the VS platform SDK folders. (http://www.opengl.org/developers/documentation/glut/)
(6) Install the DirectX 9.0b or later SDK (http://msdn.microsoft.com/library/default.asp?url=/downloads/list/directx.asp). If you are using VS6, you must use 9.0b as DirectX 9.0c no longer includes support for VS6.
(7) (DirectX 9.0c October 2004 or later only). Download the DirectX SDK Extras package , and mov the DirectShow\Samples folder into the top-level of the installed SDK path.
(8) Run the batch file {ARToolKit}\Configure.win32.bat to create include/AR/config.h.
(9) Open the ARToolKit.sln file (Visual Studio .NET 2003) or ARToolkit.dsw file (Visual Studio 6).
(10) Open the Visual Studio search paths settings (Tools->Options->Directories for VS.6, or Tools->Options->Projects->VC++ Directories for VS.NET) and add the DirectX SDK Includes\ path and the DirectX Samples\C++\DirectShow\BaseClasses\ path to the top of the search path for headers, and the DirectX SDK Lib\ path to the top of the search path for libraries.
(11) (Optional, if rebuilding DSVideoLib only). Build the DirectShow base classes strmbase.lib and strmbasd.lib. (More information can be found at Thomas Pintarics homepage for DSVideoLib (http://www.ims.tuwien.ac.at/~thomas/dsvideolib.php)).
(12) Build the toolkit.


Building on Linux / SGI Irix.
-----------------------------
Assuming the ARToolKit-2.69.tar.gz file is in your home directory, open a terminal window and type the following commands:
	cd ~
	tar zxvf ARToolKit-2.69.tar.gz
	cd ARToolKit-2.69
	./Configure
	make
Following a successful build, to run a binary such as simpleTest, add these commands:
	cd bin
	./simpleTest
	
You will be prompted as to which of the three Linux video drivers you wish to use at the Configure step.


Building on Mac OS X.
---------------------------
You will need to have Apple's Developer Tools installed in order to build ARToolKit. These can be downloaded freely from http://developer.apple.com. The library can either be built using (a) the XCode IDE or (b) from the command line:

(a) Unpack the archive to a convenient location using StuffIt Expander, and open the ARToolKit.xproj file in XCode. (The Configure target selects accelerated and rectangular texturing by default. If you wish to change these defaults, run the ./Configure script as in option (b) below.)
XCode will build the binaries in ARToolKit/bin. This must also be the working directory when the binaries are run, but by default XCode runs executables with the working directory set to Build/. In order for the binaries to be able to be run from within XCode you will need to edit the "Working directory path" of the executable to point the installed ARToolKit/bin directory on your system.

(b) Drop the ARToolKit-2.69.tar.gz file onto the Desktop and then open a Terminal window and type the following commands:
	cd ~/Desktop
	tar zxvf ARToolKit-2.69.tar.gz
	cd ARToolKit-2.69
	./Configure
	make
Following a successful build, to run a binary such as simpleTest, add these commands:
	cd bin
	./simpleTest
	
Mac OS X 10.3 and later include an inbuilt IIDC FireWire camera driver. USB camera drivers and Firewire camera drivers for pre-10.3 systems can be obtained from ioXperts at http://www.ioxperts.com.


Latest changes.
---------------
- Added libARvrml and simpleVRML example. Build instructions are in the works.
- Turn off texturing in gsub/gsub_lite when argDrawMode/arglDrawmode is AR_DRAW_BY_GL_DRAW_PIXELS.
- Added new examples collide, loadMultiple, paddle, paddleDemo, paddleInteraction, range and twoView.
- Added new utilities for testing graphics and video without marker recognition, graphicsTest and videoTest.
- VideoLinuxV4L patch by Uwe Woessner for YUV support (at last!)
- VideoLinuxV4L patches by Raphael Grasset for improved camera parameter control and compressed jpeg video stream to support Sony Eyetoy camera.
- VideoLinux1394Cam patch by Wayne Piekarski for improved lib1394 support, including multiple lib1394 versions.
- Preliminary API documentation by Raphael Grasset.


Previous changes.
-----------------
Please see the file ChangeLog.txt.

--
EOF