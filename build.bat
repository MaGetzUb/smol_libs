@echo off

cl.exe /Zi /EHsc /nologo /Fo:%1\build\%2.obj %3 /link comdlg32.lib /OUT:%1\build\%2.exe


REM "command": "cl.exe",
REM "args": [
REM     "/Zi",
REM     "/EHsc",
REM     "/nologo",
REM     "/Fo:${fileDirname}\\build\\${fileBasenameNoExtension}.obj",
REM     "${file}",
REM     "/link",
REM     "comdlg32.lib",
REM     "/OUT:${fileDirname}\\build\\{fileBasenameNoExtension}.exe"
REM ],
