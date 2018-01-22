# SXWB_Tiled_BG_Importer
Imports PNG images to NCGR, NCLR, and NSCR files. Supports multi-palettes and tiled images.

Limitations/To-Do
1. Need to allow for true multipalette. Currently only allows for detection of the first 16 colors. This was done because the the particular needs of the project do not require more than this.

2. Has only been tested on SWXB opening graphics.

3. User needs to be aware of the VRAM availablilty when trying to import images. If the number of non-transparent tiles are too many, the image will not be guaranteed to show.
