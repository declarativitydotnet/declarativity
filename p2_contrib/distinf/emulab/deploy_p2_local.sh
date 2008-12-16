#!/bin/bash

echo "Killing old files"
rm -rf /p2/*

echo "Copying the tar file"
cp /proj/P2/distinf/p2.tar.gz /p2/.

echo "Uncompressing the tar file"
mkdir /p2/distinf
cd /p2/distinf
tar zxf /p2/p2.tar.gz
