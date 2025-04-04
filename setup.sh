#!/bin/sh

echo -e ""
echo -e "******************************************************"
echo -e "*       Search for Higgs Dalitz decay at CMS         *"
echo -e "*                  H -> g*g -> eeg                   *"
echo -e "*   Developed by Cheng-Han Wu, cheng-han.wu@cern.ch  *"
echo -e "******************************************************"
echo -e ""

dir="./build"
if [ -d "$dir" ]; then
    rm -r $dir
fi
mkdir $dir
cd $dir

echo -e "1. Build the project for develpment: " $dir "\n"

echo -e "2. Construct the develpment project: HDalitzEle\n"
cmake .. -DMAKE_DEVELOPE=ON

echo -e ""
echo -e "3. Compile the shared library in the project\n"
make -j10
