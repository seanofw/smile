#!/bin/sh

sed -e 's/ToolsVersion=\"4\.0\"/ToolsVersion=\"12\.0\"/;s/<PlatformToolset>v110<\/PlatformToolset>/<PlatformToolset>v120<\/PlatformToolset>/' < SmileLib.vcxproj > temp.vcxproj
mv SmileLib.vcxproj SmileLib.vcxproj.old
mv temp.vcxproj SmileLib.vcxproj

