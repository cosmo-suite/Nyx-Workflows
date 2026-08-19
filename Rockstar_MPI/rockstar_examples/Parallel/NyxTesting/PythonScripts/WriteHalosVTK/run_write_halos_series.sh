#!/bin/bash

output="halo.series"

files_list=""
count=0

for file in $(printf "%s\n" halo_*.vtk | sort -V); do
    files_list+="    { \"name\": \"$file\", \"time\": $count },"
    files_list+=$'\n'
    ((count++))
done

# Remove trailing comma
files_list="${files_list%,}"

cat > "$output" <<EOF
{
  "file-series-version": "1.0",
  "files": [
$files_list
  ]
}
EOF

echo "JSON structure has been written to $output"
