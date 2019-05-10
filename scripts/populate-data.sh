#!/usr/bin/env bash

if [[ "$(basename $(pwd))" == "scripts" ]]
then
    pushd .. &> /dev/null
fi

if [[ -d data/.git ]]
then
    echo "Data dir exists and its a git repo"
    pushd data &> /dev/null
    git pull
    popd &> /dev/null
elif [[ -d data ]]
then
    echo "Data dir exists but its not a git repo, please remove it."
    popd &> /dev/null
    exit 1
else
    echo "Cloning data"
    git clone gogs@git.martianoids.com:imsai/zxespectrum-data.git data
    cp data/boot.cfg.orig data/boot.cfg
fi

popd &> /dev/null
exit 0
