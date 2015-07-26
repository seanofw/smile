#!/bin/sh

sed -e 's/ToolsVersion=\"4\.0\"/ToolsVersion=\"12\.0\"/;s/<PlatformToolset>v110<\/PlatformToolset>/<PlatformToolset>v120<\/PlatformToolset>/' < SmileLibTests.vcxproj > temp.vcxproj
mv SmileLibTests.vcxproj SmileLibTests.vcxproj.old
mv temp.vcxproj SmileLibTests.vcxproj

