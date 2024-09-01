# ppm2gba
This is a simple converter for taking an image in PPM format and converting it to C source that can be referred to by GBA homebrew. Please note at current the converter is very minimal and only supports Mode 3 and command line options are the absolute minimum.

# Why?
Other GBA-specific converters only support PNG and JPEG, and ImageMagick doesn't support the pixel format the GBA expects (RGB555). Maybe somebody will need that specificity, and now it's out there.

# Building
`gcc -o ppm2gba ppm2gba.c`

# License
This project is licensed under the zlib license.
