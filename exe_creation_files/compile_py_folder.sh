#!/bin/bash

folder=$1

if [[ ! -d $folder ]]; then
    echo "Error: Directory '\$folder' does not exist."
fi

for pyfile in $folder/*.py; do
    if [[ -f $pyfile ]]; then
        filename=$(basename "$pyfile" .py)

            # Extract depth and width using regex
            layers=$(echo "$filename" | grep -oP '\d+(?=layers)')
            width=$(echo "$filename" | grep -oP '\d+(?=width)')

            # Fallback in case the filename doesn't match expected pattern
            if [[ -z "$layers" || -z "$width" ]]; then
                echo "Skipping $filename: could not extract layers and width."
                continue
            fi

            out_name="mlp_l${layers}_w${width}"

            echo "Compiling $pyfile into binary named '$out_name'..."
            pyinstaller --onefile --name "$out_name" "$pyfile"
    fi
done

echo "All Python files in '\$folder' have been compiled."
