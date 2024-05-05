#!/bin/bash
# Batch convert the text encoding of source code files.
# See below website about NULLGLOB for handing null pattern matching searches.
# From https://superuser.com/questions/1310211/sh-syntax-to-handle-zero-files-matching-a-wildcard-as-well-as-more

#FROM_ENCODING="iso-8859-1"
FROM_ENCODING="WINDOWS-1252"
TO_ENCODING="us-ascii"
ARCHIVEDIR="ArchiveConv"

#conversion method
CONVERT=" iconv  -f   $FROM_ENCODING  -t   $TO_ENCODING"

SAVED_NULLGLOB=$(shopt -p | grep nullglob)
shopt -s nullglob

#make a backup copy of the files to be converted 
if [ ! -d "$ARCHIVEDIR" ]; then
   mkdir "$ARCHIVEDIR"
   for file in *.{h,c,cpp}; do
      mv "$file" "./$ARCHIVEDIR/"
   done   
fi

#loop to convert all the archived folders files into this current folder.
# WARNING... conversion will drop any invalid characters from the output, e.g. any 0xA9 copyright symbol in a utf-8 file
for file in ./$ARCHIVEDIR/*; do
   $CONVERT "$file" -c -o "${file##*/}"
done

eval "$SAVED_NULLGLOB"

exit 0

