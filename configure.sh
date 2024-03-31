#!/bin/bash

gcc -o shell shell.c ./my_libs/client_lib.c ./my_libs/server_lib.c

if [ $? -eq 0 ]; then
    echo "------------------ Compilation successful. ------------------"

    # Check if there are any command-line arguments
    if [ $# -gt 0 ]; then
        # Run the compiled program with command-line arguments
        ./shell "$@"
    else
        # Run the compiled program without any command-line arguments
        ./shell
    fi
else
    echo "Compilation failed. Please fix the errors."
fi
