#!/bin/bash

result_file="_afbl_all.sql"
out_folder="\$root\$/AFBL"
                      
for f in *.afb
do 
    echo "Processing $f"
   ./file2pgsql $f $f.sql "$out_folder"
done

if [ -f $result_file ];
then
    echo "deleting old file $result_file..."
    rm $result_file
fi
cat *.sql > $result_file
