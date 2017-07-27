@echo off

ECHO Analyzing test coverage...
ECHO.

OpenCppCoverage --sources . --export_type html:CoverageReport -- smilelibtests\bin\Win32-Debug\SmileLibTests.exe

