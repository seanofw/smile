#!/bin/sh

sed -e 's/ToolsVersion=\"12\.0\"/ToolsVersion=\"4\.0\"/;s/<PlatformToolset>v120<\/PlatformToolset>/<PlatformToolset>v110<\/PlatformToolset>/' < SmileLibTests.vcxproj > temp.vcxproj
mv SmileLibTests.vcxproj SmileLibTests.vcxproj.old
mv temp.vcxproj SmileLibTests.vcxproj

