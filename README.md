```sh
cmake \
    -B build \
    -S . \
    --toolchain toolchain_gnu.cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_BUILD_TYPE=Debug
```

```sh
cmake --build build --target=wholth
```

```sh
cp ~/Projects/wholth/build/libwholth.dylib /usr/local/lib/.    
```
