#!/bin/bash

# Check if the script is running from the src folder
if [ "$(basename "$PWD")" == "src" ]; then
  cd ..
fi

make clean

# Define the list of autoreconf-related files and folders to remove
files_and_folders=(
  "autom4te.cache"
  "aclocal.m4"
  "Makefile.in"
  "configure"
  "config.log"
  "config.status"
  "depcomp"
  "install-sh"
  "missing"
  "compile"
  "test-driver"
  "ar-lib"
  "Makefile"
  ".deps"
  "atconfig"
  "*.err"
)

# Function to recursively delete files and folders
delete_autoreconf_files() {
  local folder="$1"
  for item in "${files_and_folders[@]}"; do
    find "$folder" -name "$item" -exec rm -rf {} + -print
  done
}

# Recursively clean src and tests directories
delete_autoreconf_files "."

echo "Cleanup completed!"
