#!/usr/bin/env bash

# stdbuf -oL 

./inner_test.sh | {
  while IFS= read -r line
  do
    echo "LINE:"
    echo "$line"
  done
}
