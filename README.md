# LUT-packer
Maps 5 LUTs into minimum number of 6-LUTs. [Uses blossom algorithm to give an optimal solution](https://website.ahmadk.ca/LUT_packer/). <br>

## Usage
Create an executable using gcc and pass a blif file as an argument to the executable. Pipe the output to a file to store it. 

```bash
gcc lut_packing.c -o lut_packer
```
```
./lut_packer sample_blif_files/alu4.blif
```
or
```
./lut_packer sample_blif_files/alu4.blif > output.txt
```
