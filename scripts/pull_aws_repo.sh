#!/bin/bash

repo_prefix="https://github.com/awslabs/aws-c-"
patch_prefix="seabmc_"
patch_postfix=".patch"
repo_list=("cal" "common" "compression" "io" "sdkutils")
hash_list=("3d4c08b60ffa8698cda14bb8d56e5d6a27542f17" "15a25349d59852e2655c0920835644f2eb948d77" "94f748ae244c72a2e42c63818259ce8877ad6a5a" "df64f57feb63ab1a489ded86a87b756a48c46f35" "fd8c0ba2e233997eaaefe82fb818b8b444b956d3")

SCRIPT_DIR="$(dirname "$(realpath "$0")")"
PARENT_DIR=$(dirname "$SCRIPT_DIR")
PATCH_DIR="$PARENT_DIR/patch"

for i in "${!repo_list[@]}"; do
    (
        # clone each repo
        git clone "${repo_prefix}${repo_list[$i]}.git" || { echo "Failed to clone ${repo_list[$i]}"; exit 1; }
        cd "aws-c-${repo_list[$i]}" || { echo "Failed to enter directory aws_c_${repo_list[$i]}"; exit 1; }
        # checkout to the specific hash
        git checkout "${hash_list[$i]}" || { echo "Failed to checkout ${hash_list[$i]}"; exit 1; }
        # apply the patch
        git apply "${PATCH_DIR}/${patch_prefix}${repo_list[$i]}${patch_postfix}" || { echo "Failed to apply patch for ${repo_list[$i]}"; exit 1; }
        cd ..
    )
done