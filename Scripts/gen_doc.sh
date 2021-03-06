#!/bin/bash

echo -n "Checking for doxygen... "
EXE_PATH=$(which astyle)
if [ ! -x "$EXE_PATH" ] ; then
    echo "NOT FOUND"
    echo "    Make sure you download doxygen before using this script."
    echo "    See README.md for more info. Exiting."
    read -p "Press [Enter] to continue. "
    exit
else
    echo "FOUND"
fi

pushd . > /dev/null
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd ${DIR}/..
echo -n "Current directory is "; pwd

doxygen Scripts/doxygen.cfg

popd > /dev/null
