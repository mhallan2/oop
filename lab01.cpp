// Класс для работы с цветовыми моделями
#include <iostream>
#include <cmath>
#include <algorithm>

using namespace std;

constexpr double rgb_max = 255.0;
constexpr double full_circle = 360.0;
constexpr double degrees_per_sector = 60.0;
constexpr double red_sector_shift = 0.0;
constexpr double green_sector_shift = 2.0;
constexpr double blue_sector_shift = 4.0;

struct HSV {
    double h; // 0..360
    double s; // 0..1
    double v; // 0..1
};

struct RGB {
    int r; // 0..255
    int g; // 0..255
    int b; // 0..255
};

class Color {

private:
    int red = 0;
    int green = 0;
    int blue = 0;

    static HSV RGBtoHSV(int r, int g, int b) {
        HSV result;

        double red_norm = r / rgb_max;
        double green_norm = g / rgb_max;
        double blue_norm = b / rgb_max;

        double max_value = std::max({red_norm, green_norm, blue_norm});
        double min_value = std::min({red_norm, green_norm, blue_norm});
        double delta = max_value - min_value;

        double saturation =
            (max_value == 0.0) ? 0.0 : delta / max_value;

        double hue;

        if (delta == 0.0) {
            hue = 0.0;

        } else if (max_value == red_norm) {
            hue = degrees_per_sector *
                  ((green_norm - blue_norm) / delta + red_sector_shift);

            if (hue < 0.0) {
                hue += full_circle;
            }

        } else if (max_value == green_norm) {
            hue = degrees_per_sector *
                  ((blue_norm - red_norm) / delta + green_sector_shift);

        } else {
            hue = degrees_per_sector *
                  ((red_norm - green_norm) / delta + blue_sector_shift);
        }

        result.h = hue;
        result.s = saturation;
        result.v = max_value;

        return result;
    }

    static RGB HSVtoRGB(double h, double s, double v) {
        RGB result;

        double chroma = v * s;
        double secondary =
            chroma *
            (1.0 - fabs(fmod(h / degrees_per_sector, 2.0) - 1.0));

        double offset = v - chroma;

        double r;
        double g;
        double b;

        if (h >= 0.0 && h < degrees_per_sector) {
            r = chroma;
            g = secondary;
            b = 0.0;

        } else if (h < 2.0 * degrees_per_sector) {
            r = secondary;
            g = chroma;
            b = 0.0;

        } else if (h < 3.0 * degrees_per_sector) {
            r = 0.0;
            g = chroma;
            b = secondary;

        } else if (h < 4.0 * degrees_per_sector) {
            r = 0.0;
            g = secondary;
            b = chroma;

        } else if (h < 5.0 * degrees_per_sector) {
            r = secondary;
            g = 0.0;
            b = chroma;

        } else {
            r = chroma;
            g = 0.0;
            b = secondary;
        }

        result.r = round((r + offset) * rgb_max);
        result.g = round((g + offset) * rgb_max);
        result.b = round((b + offset) * rgb_max);

        return result;
    }

public:
    int GetRed() const {
        return red;
    }

    int GetGreen() const {
        return green;
    }

    int GetBlue() const {
        return blue;
    }

    double GetHue() const {
        HSV hsv = RGBtoHSV(red, green, blue);
        return hsv.h;
    }

    double GetSaturation() const {
        HSV hsv = RGBtoHSV(red, green, blue);
        return hsv.s;
    }

    double GetValue() const {
        HSV hsv = RGBtoHSV(red, green, blue);
        return hsv.v;
    }

    void SetRGB(int r, int g, int b) {
        if (r < 0 || r > 255 ||
            g < 0 || g > 255 ||
            b < 0 || b > 255) {

            cout << "Error: RGB values must be in range 0..255\n";
            return;
        }

        red = r;
        green = g;
        blue = b;
    }

    void SetHSV(double h, double s, double v) {
        if (h < 0.0 || h > full_circle ||
            s < 0.0 || s > 1.0 ||
            v < 0.0 || v > 1.0) {

            cout << "Error: invalid HSV values\n";
            return;
        }

        RGB rgb = HSVtoRGB(h, s, v);

        red = rgb.r;
        green = rgb.g;
        blue = rgb.b;
    }

    void SetRed(int r) {
        if (r < 0 || r > 255) {
            cout << "Error: red must be in range 0..255\n";
            return;
        }

        red = r;
    }

    void SetGreen(int g) {
        if (g < 0 || g > 255) {
            cout << "Error: green must be in range 0..255\n";
            return;
        }

        green = g;
    }

    void SetBlue(int b) {
        if (b < 0 || b > 255) {
            cout << "Error: blue must be in range 0..255\n";
            return;
        }

        blue = b;
    }
};


int main() {
    Color color;

    // 1. Задание цвета через RGB
    color.SetRGB(0, 0, 255);

    cout << "RGB:\n";
    cout << "Red: " << color.GetRed() << endl;
    cout << "Green: " << color.GetGreen() << endl;
    cout << "Blue: " << color.GetBlue() << "\n\n";

    cout << "HSV:\n";
    cout << "Hue: " << color.GetHue() << endl;
    cout << "Saturation: " << color.GetSaturation() << endl;
    cout << "Value: " << color.GetValue() << "\n\n";


    // 2. Задание цвета через HSV
    color.SetHSV(120, 1, 1);

    cout << "After SetHSV(120, 1, 1):\n";
    cout << "Red: " << color.GetRed() << endl;
    cout << "Green: " << color.GetGreen() << endl;
    cout << "Blue: " << color.GetBlue() << "\n\n";


    // 3. Проверка неправильных данных
    color.SetRGB(100, 200, 355);

    cout << "After invalid SetRGB:\n";
    cout << "Red: " << color.GetRed() << endl;
    cout << "Green: " << color.GetGreen() << endl;
    cout << "Blue: " << color.GetBlue() << "\n\n";


    // 4. Работа с константным объектом
    const Color const_color;

    cout << "Const object:\n";
    cout << "Red: " << const_color.GetRed() << endl;
    cout << "Green: " << const_color.GetGreen() << endl;
    cout << "Blue: " << const_color.GetBlue() << endl;
    cout << "Hue: " << const_color.GetHue() << endl;
    cout << "Saturation: " << const_color.GetSaturation() << endl;
    cout << "Value: " << const_color.GetValue() << endl;

    return 0;
}
