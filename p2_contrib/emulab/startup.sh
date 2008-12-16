#!/bin/sh

#logfile=$1
#echo log file: $logfile
#if [ -e $logfile ]
#then
#sudo chown $USER:P2 $logfile
#fi

echo Copying tar files
sudo cp /proj/P2/distinf/{p2,rpms}.tar.gz /tmp

echo Installing packages
cd /tmp
tar zxf rpms.tar.gz
sudo rpm -U rpms/*.rpm

#sudo yum -y install lapack
#sudo yum -y install lapack-devel
#sudo yum -y install blas
#sudo yum -y install blas-devel
#sudo yum -y install subversion
#sudo yum -y install cmake

if [ -e /dev/hda4 ]
then
	partition=/dev/hda4
elif [ -e /dev/sda4 ]
then
	partition=/dev/sda4
else
	echo "/dev/sda4 or /dev/hda4 not found! quitting"
exit 1
fi
echo Using partition: $partition

echo Creating a larger swap file
sudo dd if=$partition of=/swapfile bs=1024 count=1048576
sudo mkswap /swapfile
sudo swapon /swapfile

echo Creating a larger local file system
sudo mkfs $partition
sudo mkdir /p2
sudo mount $partition /p2
sudo chown -R $USER:P2 /p2
sudo chmod -R g+rw /p2

if [ "$(echo $HOSTNAME | grep build)" == "" ]; then
    mkdir /p2/distinf
    cd /p2/distinf
    tar zxf /tmp/p2.tar.gz
    echo "Started a run node"
else
    ln -s /proj/P2/distinf /p2/distinf
    echo "Started a build node"
fi

export OVERLOG=/p2/distinf/release/bin/runStagedOverlog
