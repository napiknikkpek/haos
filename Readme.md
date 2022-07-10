Здесь показан эффективный обрабочик столкновений объектов между собой и краями окна. Сложность O(N) для каждого столкновения. Объекты абсолютно упругие, описываются кругами заданного радиуса и имеют заданный вектор скорости. Объекты двигаются в плоскости окна и абсолютно упруго отскакивают друг от друга и от краёв окна. Массы объектов пропорциональны площади кругов, их описывающих.

Use conan as packet manager to compile
```bash
mkdir build
cd build
conan install .. -b missing -s build_type=Release
cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
cmake --build .
```
