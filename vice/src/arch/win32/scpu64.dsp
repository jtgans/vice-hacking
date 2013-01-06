# Microsoft Developer Studio Project File - Name="scpu64" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=scpu64 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "scpu64.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scpu64.mak" CFG="scpu64 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scpu64 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "scpu64 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "scpu64 - Win32 DX Release" (based on "Win32 (x86) Static Library")
!MESSAGE "scpu64 - Win32 DX Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scpu64 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libs\scpu64\Release"
# PROP Intermediate_Dir "libs\scpu64\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\msvc" /I ".\\" /I "..\..\\" /I "..\..\drive" /I "..\..\lib\p64" /I "..\..\drive\iec\c64exp" /I "..\..\userport" /I "..\..\video" /I "..\..\tape" /I "..\..\sid" /I "..\..\viciisc" /I "..\..\c64" /I "..\..\c64\cart" /I "..\..\raster" /I "..\..\monitor" /I "..\..\rs232drv" /D "WIN32" /D "NODIRECTX" /D "IDE_COMPILE" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "scpu64 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libs\scpu64\Debug"
# PROP Intermediate_Dir "libs\scpu64\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I ".\msvc" /I ".\\" /I "..\..\\" /I "..\..\drive" /I "..\..\lib\p64" /I "..\..\drive\iec\c64exp" /I "..\..\userport" /I "..\..\video" /I "..\..\tape" /I "..\..\sid" /I "..\..\c64" /I "..\..\c64\cart" /I "..\..\viciisc" /I "..\..\raster" /I "..\..\monitor" /I "..\..\rs232drv" /D "WIN32" /D "NODIRECTX" /D "IDE_COMPILE" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "scpu64 - Win32 DX Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DXRelease"
# PROP BASE Intermediate_Dir "DXRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libs\scpu64\DXRelease"
# PROP Intermediate_Dir "libs\scpu64\DXRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I ".\msvc" /I ".\\" /I "..\..\\" /I "..\..\drive" /I "..\..\lib\p64" /I "..\..\drive\iec\c64exp" /I "..\..\userport" /I "..\..\video" /I "..\..\tape" /I "..\..\sid" /I "..\..\viciisc" /I "..\..\c64" /I "..\..\c64\cart" /I "..\..\raster" /I "..\..\monitor" /I "..\..\rs232drv" /D "WIN32" /D "IDE_COMPILE" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I ".\msvc" /I ".\\" /I "..\..\\" /I "..\..\drive" /I "..\..\lib\p64" /I "..\..\drive\iec\c64exp" /I "..\..\userport" /I "..\..\video" /I "..\..\tape" /I "..\..\sid" /I "..\..\viciisc" /I "..\..\c64" /I "..\..\c64\cart" /I "..\..\raster" /I "..\..\monitor" /I "..\..\rs232drv" /D "WIN32" /D "IDE_COMPILE" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "scpu64 - Win32 DX Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DXDebug"
# PROP BASE Intermediate_Dir "DXDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libs\scpu64\DXDebug"
# PROP Intermediate_Dir "libs\scpu64\DXDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od /I ".\msvc" /I ".\\" /I "..\..\\" /I "..\..\drive" /I "..\..\lib\p64" /I "..\..\drive\iec\c64exp" /I "..\..\userport" /I "..\..\video" /I "..\..\tape" /I "..\..\sid" /I "..\..\c64" /I "..\..\c64\cart" /I "..\..\viciisc" /I "..\..\raster" /I "..\..\monitor" /I "..\..\rs232drv" /D "WIN32" /D "IDE_COMPILE" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I ".\msvc" /I ".\\" /I "..\..\\" /I "..\..\drive" /I "..\..\lib\p64" /I "..\..\drive\iec\c64exp" /I "..\..\userport" /I "..\..\video" /I "..\..\tape" /I "..\..\sid" /I "..\..\c64" /I "..\..\c64\cart" /I "..\..\viciisc" /I "..\..\raster" /I "..\..\monitor" /I "..\..\rs232drv" /D "WIN32" /D "IDE_COMPILE" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "scpu64 - Win32 Release"
# Name "scpu64 - Win32 Debug"
# Name "scpu64 - Win32 DX Release"
# Name "scpu64 - Win32 DX Debug"
# Begin Source File

SOURCE="..\..\scpu64\scpu64.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64-cmdline-options.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64cpu.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64gluelogic.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64mem.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64meminit.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64memsnapshot.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64-resources.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64rom.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64-snapshot.c"
# End Source File
# Begin Source File

SOURCE="..\..\scpu64\scpu64stubs.c"
# End Source File
# End Target
# End Project
