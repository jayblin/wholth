```sh
cmake -B build -S . \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_INSTALL_PREFIX=/Users/<USER>/Projects/wholth/build \
    -DVULKAN_SDK_INCLUDE_DIR=/Users/<USER>/VulkanSDK/1.3.250.1/MoltenVK/include \
    -DVULKAN_SDK_LIBRARY_PATH=/Users/<USER>/VulkanSDK/1.3.250.1/macOS/Frameworks/vulkan.framework/vulkan \
    -DDEFAULT_FONT_PATH=/Users/<USER>/<FONT>.ttf
```
