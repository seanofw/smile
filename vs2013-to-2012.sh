#!/bin/sh

pushd SmileC/smilelib; ./vs2013-to-2012.sh; popd
pushd SmileC/smilelibtests; ./vs2013-to-2012.sh; popd

