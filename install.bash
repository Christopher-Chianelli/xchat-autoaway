#!/bin/bash
if which gnome-screensaver-command > /dev/null
then
    gcc -Wl,--export-dynamic -Wall -O1 -shared -fPIC autoaway.c -o autoaway.so
else
    echo "Dependency 'gnome-screensaver-command' not found. Please rerun make after installing it."
    exit 1
fi

