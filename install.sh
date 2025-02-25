#!/bin/bash

mkdir -p python_packages/
mkdir -p ollama_whl/

cd ollama_whl
pip download ollama

for file in *.whl; do
    unzip -o "$file" -d .
done

export PYTHONPATH=$HOME/python_packages:$PYTHONPATH
echo 'export PYTHONPATH=${pwd}/python_packages:$PYTHONPATH' >> ~/.bashrc
source ~/.bashrc

