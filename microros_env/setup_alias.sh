#!/bin/bash
SCRIPT_PATH="/home/loriek/env/microros_env/microros.py"
ALIAS_LINE="alias microros='python3 $SCRIPT_PATH'"

if ! grep -q "$ALIAS_LINE" ~/.bashrc; then
    echo "Adding microros alias to ~/.bashrc"
    echo "$ALIAS_LINE" >> ~/.bashrc
    echo "Done! Please run 'source ~/.bashrc' to activate the alias."
else
    echo "Alias already exists in ~/.bashrc"
fi
