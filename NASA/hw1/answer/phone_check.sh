#! /usr/bin/env bash

if [ $# -gt 0 ]; then
	echo $1 | sed -En '/^09[0-9]{2}((-[0-9]{3}-{0,1}[0-9]{3})|([0-9]{6}))$/p'
fi
