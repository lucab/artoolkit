Read me for ARToolKit-2.69.
===========================


Contents.
---------
About this archive.
Building on Windows.
Building on Linux / SGI Irix.
Building on Mac OS X.
Changes in version 2.69.
Changes in version 2.68.2.
Changes in version 2.68.1.
Changes in version 2.68.


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
 

Changes in version 2.69.
--------------------------
- Windows builds still depend on DSVideoLib, but DSVideoLib is no longer included in source-only ARToolKit releases and must be downloaded separately.
- Fixed bugs identified by Raphael Grasset: typo 'DEFALUT' in config.h.in and arUtil.c, return values in arMultiActivate.c, implementation of arUtilSleep for Win32.
- Fixed help text in arVideoDispOption for VideoWin32DirectShow.
- Fixed bug in ar2VideoClose() identified by Joerg Hedrich (http://www.hitl.washington.edu/artoolkit/mail-archive/message-thread-00690-possible-bug-in-libARvid.html).
- For builds under Visual Studio 6, all projects are now in one workspace (.dsw) file, and builds now go into lib/ and bin/ rather than lib.vc60/ and bin.vc60/.
- For debug builds under Visual Studio, libraries and binaries now have the character 'd' appended to the name.
- For builds under Visual Studio 6, all libraries now use multithreaded code generation model.
- For builds under Apple XCode, creation of config.h is now performed by a script run by the aggregate target 'Configure'.
- Changed the '_' character in releases to a '-' character to be more compatible with Unix packaging system conventions.
- Fix bug identified by Alexandre Gillet: arLabelling.c allocates buffer for arImage with maximum dimension 512 pixels.


Changes in version 2.68.2.
--------------------------
- VideoWin32DirectShow uses DSVideoLib 0.0.4 (was 0.0.3). The samples now correct the vertical orientation of video from DSVideoLib by specifying the "flipV" parameter by default. An alternative is to use the WDM capture settings dialog 'Mirror horizontally' parameter and rotate the camera 180 degrees.
- Rewrite of Windows AR/video.h interface to follow same scheme for multiple cameras as used on other platforms (i.e. using the ar2Video*() set of functions).
- All platforms now support the arVideoDispOptions() call, which prints acceptable options for arVideoOpen()'s configuration string to the standard output. New applications using ARToolKit should provide a way for a user to specify a preferred configuration string at runtime (rather than compile time), and to see the output of arVideoDispOptions() so that he or she can learn possible configuration strings.
- Optimisations in arLabelling.c and arGetCode.c by Thomas Pintaric.
- gsub libraries compile correctly on platforms with older OpenGL headers. Removed extraneous glPixelStorei(GL_UNPACK_IMAGE_HEIGHT,...) calls.
- New function arglDispImageStateful() in gsub_lite for special effects.
- Removed the obsolete graphicsTest example.


Changes in version 2.68.1.
--------------------------
- Mac video settings dialog can now be disabled by adding "-nodialog" to options string passed to arVideoOpen().
- Fixed some build bugs.


Changes in version 2.68.
--------------------------
Version 2.65 of ARToolKit incorporated the enhanced Windows video capture library created by Thomas Pintaric (http://www.ims.tuwien.ac.at/~thomas/artoolkit.php). Version 2.68 is based on 2.65f with the following changes:
- New gsub_lite library designed to allow easier use of ARToolKit in existing applications, plus complete documentation in HTML format in doc/gsub_lite.
- New high-performance Macintosh video capture library by Philip Lamb.
- Support throughout the toolkit for three extra pixel formats including two packed-pixel YCbCr types (Mac pixel format codes 'ARGB', '2vuy', 'yuvs'). 
- Several minor changes for compiler correctness, such as return values from main() and const declarations in some function parameter lists.
- Fix for bug in arGetTransMat2.c identified by Joerg Stoecklein (http://www.hitl.washington.edu/artoolkit/mail-archive/message-thread-00627--variable--s1--not-initi.html).
- Fix for bug in VideoLinuxDV/video.c identified by Tobias Laubach (http://www.hitl.washington.edu/artoolkit/mail-archive/message-thread-00626-Segmentation-error---fix.html).

--
EOF