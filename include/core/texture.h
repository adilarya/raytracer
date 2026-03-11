#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include "math/vec3.h"

template <typename T>
struct Texture {
    using Color3 = Vec3<T>;
    // attributes
    int width, height;
    std::vector<std::vector<Color3>> pixels; // 2D array of colors

    // constructors
    Texture() : width(0), height(0), pixels() {} // default constructor for empty texture
    
    // methods
    bool load(const std::string& filename) {
        FILE* file = fopen(filename.c_str(), "r");
        if (file == NULL) {
            std::cerr << "[ERROR] Could not open file: " << filename << std::endl;
            return false;
        }

        // reading a PPM file (P3 format)
        char format[3];
        if (fscanf(file, "%2s", format) != 1) {
            std::cerr << "[ERROR] Failed to read image format from file: " << filename << std::endl;
            fclose(file);
            return false;
        }

        if (strcmp(format, "P3") != 0) {
            std::cerr << "[ERROR] Unsupported image format: " << format << ". Only P3 PPM format is supported." << std::endl;
            fclose(file);
            return false;
        }

        // going until we find the width and height (taking comments into account)
        do {
            char c = fgetc(file);
            if (c == '#') {
                // skip the comment line
                while (fgetc(file) != '\n');
            } else {
                ungetc(c, file); // put the character back for reading
                break;
            }
        } while (true);

        if (fscanf(file, "%d %d", &width, &height) != 2) {
            std::cerr << "[ERROR] Failed to read image dimensions from file: " << filename << std::endl;
            fclose(file);
            return false;
        }

        // double check dimensions
        if (width <= 0 || height <= 0) {
            std::cerr << "[ERROR] Invalid image dimensions: " << width << "x" << height << " in file: " << filename << std::endl;
            fclose(file);
            return false;
        }

        int max_color_value;
        if (fscanf(file, "%d", &max_color_value) != 1) {
            std::cerr << "[ERROR] Failed to read max color value from file: " << filename << std::endl;
            fclose(file);
            return false;
        }

        // validate max color value
        if (max_color_value <= 0) {
            std::cerr << "[ERROR] Invalid max color value: " << max_color_value << " in file: " << filename << std::endl;
            fclose(file);
            return false;
        }

        // allocate memory for pixels
        pixels.resize(height, std::vector<Color3>(width));

        // read pixel data
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                int r, g, b;
                if (fscanf(file, "%d %d %d", &r, &g, &b) != 3) {
                    std::cerr << "[ERROR] Failed to read pixel data from file: " << filename << std::endl;
                    fclose(file);
                    return false;
                }

                pixels[i][j] = Color3(static_cast<T>(r) / max_color_value, static_cast<T>(g) / max_color_value, static_cast<T>(b) / max_color_value);
            }
        }

        fclose(file);
        return true;
    }

    Color3 sample(T u, T v) const {
        if (width == 0 || height == 0) {
            return Color3(0, 0, 0); // return black for empty texture
        }

        // wrap texture coordinates
        u = u - std::floor(u);
        v = v - std::floor(v);

        // convert to pixel coordinates
        T x = u * (width - T(1));
        T y = v * (height - T(1));

        int x0 = static_cast<int>(std::floor(x));
        int x1 = static_cast<int>(std::ceil(x));
        int y0 = static_cast<int>(std::floor(y));
        int y1 = static_cast<int>(std::ceil(y));

        // clamp pixel coordinates to valid range
        x0 = std::clamp(x0, 0, width - 1);
        x1 = std::clamp(x1, 0, width - 1);
        y0 = std::clamp(y0, 0, height - 1);
        y1 = std::clamp(y1, 0, height - 1);

        // get the four surrounding pixel colors
        Color3 c00 = pixels[y0][x0];
        Color3 c10 = pixels[y0][x1];
        Color3 c01 = pixels[y1][x0];
        Color3 c11 = pixels[y1][x1];

        // compute the weights for interpolation
        T wx = x - x0;
        T wy = y - y0;

        // perform bilinear interpolation
        Color3 c0 = (T(1) - wx) * c00 + wx * c10;
        Color3 c1 = (T(1) - wx) * c01 + wx * c11;
        return (T(1) - wy) * c0 + wy * c1;
    }
};

#endif
