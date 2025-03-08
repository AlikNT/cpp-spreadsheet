# Spreadsheet

![C++17](https://img.shields.io/badge/С++17-00599C)
![ANTLR](https://img.shields.io/badge/ANTLR-222222)
![CMake](https://img.shields.io/badge/CMake-D1242F)


### Description
This is a diploma project of Electronic table - a simpified analogue of Microsoft Excel. The project is implemented in `C++` using the program `ANTLR`. The program runs tests located in `main.cpp`. The program can be used as a library implementing work with an electronic table. 

### Build
```Bash
mkdir -p /build && cd /build && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
```

### Run
```Bash
/build/spreadsheet
```
