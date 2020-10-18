#!/bin/bash
set -ex

NUM=BBC
DIRNAME=project1_team"$NUM"_v0


mkdir -p tmp/$DIRNAME tmp/$DIRNAME/code
gimli -file README.md -outputdir tmp/$DIRNAME -outputfilename project1_team"$NUM"
cp code tmp/$DIRNAME -r

zip -r $DIRNAME.zip tmp/$DIRNAME
