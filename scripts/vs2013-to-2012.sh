#!/bin/sh

sed -e 's/ToolsVersion=\"12\.0\"/ToolsVersion=\"4\.0\"/;s/<PlatformToolset>v120<\/PlatformToolset>/<PlatformToolset>v110<\/PlatformToolset>/' < SmileLib.vcxproj > temp.vcxproj
mv SmileLib.vcxproj SmileLib.vcxproj.old
mv temp.vcxproj SmileLib.vcxproj

