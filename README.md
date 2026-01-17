# Genetic Photo Reconstruct
<img width="1072" height="326" alt="image" src="https://github.com/user-attachments/assets/1541a15d-27ce-476e-afd3-08d350be5467" />
Example: Recreating a dog photo from a cat photo

## Features
* **Genetic Algorithm:** Uses mutation and hill-climbing to evolve image accuracy.
* **Optimized Rendering:** Custom `sf::VertexArray` batching reduces draw calls from thousands to one per frame.
* **Configurable:** CLI support for inputs, DNA length, and output paths.

## Build Instructions (CMake)

Requires **C++17** and **SFML 3.0+**.

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Usage
```bash
./evolve [options]
```
Options:
```
  -i, --input <path>   Path to source image (default: ./assets/target.png)
  -s, --sprite <path>   Path to sprite image (default: input)
  -o, --output <path>  Path to save result (default: output.png)
  -d, --dna <number>   Number of shapes to draw (default: 500)
```
The result is saved to output path when close window.
