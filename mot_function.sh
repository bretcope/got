function mot() {
#  temp_script=$(mktemp)
#  echo "Created temp: $temp_script"
#  trap "rm -f $temp_script" RETURN
#  chmod 600 "$temp_script"

  output=$(/c/bret/rust/mot/target/debug/motcli.exe -s "$@")
  exit_code="$?"

  if [[ "$exit_code" == "2" ]]
  then
    cd "$output"
  else
    echo -n "$output"
  fi

#  /c/bret/rust/mot/target/debug/motcli.exe "$@" | {
#    while IFS= read -r line
#    do
#      if [[ $line =~ ^::EXEC::(.*)$ ]]; then
#        echo "${BASH_REMATCH[1]}" >> "$temp_script"
#      else
#        echo "$line"
#      fi
#    done
#  }

#  echo "cd /c/york/it" >> "$temp_script"
#
#  while IFS= read -r line
#  do
#    eval "$line"
#  done < "$temp_script"
}