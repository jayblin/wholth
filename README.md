```sh
cmake \
    -B build \
    -S . \
    --toolchain toolchain_gnu.cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

```sh
cmake --build build --target=wholth
```
