#include <cstddef>
#include <iostream>
#include <new>

class Vector {
  public:
    Vector() : data_(nullptr), size_(0), owns_memory_(true) {}

    explicit Vector(int size) : Vector(size, 0.0) {}

    Vector(int size, double value) : data_(nullptr), size_(0), owns_memory_(true) {
        if (size >= 0) {
            size_ = size;
            data_ = Allocate(size_);

            if (data_ == nullptr && size_ != 0) {
                size_ = 0;
            } else {
                Fill(data_, size_, value);
            }
        }
    }

    Vector(double *external_data, int size) : data_(nullptr), size_(0), owns_memory_(false) {
        if (size >= 0 && (size == 0 || external_data != nullptr)) {
            data_ = external_data;
            size_ = size;
        }
    }

    Vector(const Vector &other) : data_(nullptr), size_(0), owns_memory_(other.owns_memory_) {
        if (other.owns_memory_) {
            data_ = Allocate(other.size_);
            if (data_ != nullptr || other.size_ == 0) {
                size_ = other.size_;
                Copy(data_, other.data_, size_);
            }
        } else {
            data_ = other.data_;
            size_ = other.size_;
        }
    }

    ~Vector() {
        if (owns_memory_) {
            delete[] data_;
        }
    }

    int Size() const { return size_; }

    bool OwnsMemory() const { return owns_memory_; }

    bool SetElement(int index, double value) {
        if (index >= size_) {
            return false;
        }
        data_[index] = value;
        return true;
    }

    double GetElement(int index) const {
        if (size_ == 0) {
            return 0.0;
        }

        if (index >= size_) {
            index = size_ - 1;
        }

        return data_[index];
    }

    double Dot(const Vector &other) const {
        if (size_ != other.size_) {
            std::cout << "Ошибка: размеры векторов различаются\n";
            return 0.0;
        }

        double result = 0.0;
        for (int i = 0; i < size_; ++i) {
            result += data_[i] * other.data_[i];
        }
        return result;
    }

    void Link(double *external_data, int size) {
        if (external_data == data_)
            return;

        if (size < 0 || (size != 0 && external_data == nullptr)) {
            std::cout << "Ошибка: неверные данные для Link\n";
            return;
        }

        if (owns_memory_) {
            delete[] data_;
        }

        data_ = external_data;
        size_ = size;
        owns_memory_ = false;
    }

    // Создаёт собственную копию внешнего массива
    void Unlink() {
        if (owns_memory_) {
            return;
        }

        double *new_data = Allocate(size_);
        if (new_data == nullptr && size_ != 0) {
            std::cout << "Ошибка: не удалось выделить память для Unlink\n";
            return;
        }

        Copy(new_data, data_, size_);
        data_ = new_data;
        owns_memory_ = true;
    }

  private:
    double *data_;
    int size_;
    bool owns_memory_;

    static double *Allocate(int size) {
        if (size == 0) {
            return nullptr;
        }
        return new (std::nothrow) double[size];
    }

    static void Fill(double *data, int size, double value) {
        for (int i = 0; i < size; ++i) {
            data[i] = value;
        }
    }

    static void Copy(double *destination, const double *source, int size) {
        for (int i = 0; i < size; ++i) {
            destination[i] = source[i];
        }
    }
};

void Print(const char *name, const Vector &vector) {
    std::cout << name << " (size=" << vector.Size() << ", owns=" << std::boolalpha
              << vector.OwnsMemory() << "): [";

    for (int i = 0; i < vector.Size(); ++i) {
        double value = vector.GetElement(i);
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << value;
    }
    std::cout << "]\n";
}

int main() {
    Vector empty;
    Vector zeros(3);
    Vector filled(4, 2.5);

    zeros.SetElement(0, 1.0);
    zeros.SetElement(1, 2.0);
    zeros.SetElement(2, 3.0);

    Print("empty", empty);
    Print("zeros", zeros);
    Print("filled", filled);

    // Копирование владеющего вектора создаёт независимую копию
    Vector owning_copy(zeros);

    zeros.SetElement(0, 100.0);

    Print("zeros", zeros);
    Print("owning_copy", owning_copy);

    // Копирование невладеющего вектора копирует адрес внешнего массива
    double external[] = {10.0, 20.0, 30.0};

    Vector linked(external, 3);
    Vector linked_copy(linked);

    linked_copy.SetElement(1, 200.0);

    Print("linked", linked);
    Print("linked_copy", linked_copy);

    // Скалярное произведение
    Vector left(3, 2.0);
    Vector right(3, 4.0);

    double dot_result = left.Dot(right);

    std::cout << "Dot = " << dot_result << "\n";

    // Link + Unlink
    double second_external[] = {-1.0, -2.0, -3.0};

    Vector reusable(2, 8.0);
    Print("reusable before Link", reusable);

    reusable.Link(second_external, 3);
    reusable.SetElement(0, 123.0);

    Print("reusable after Link", reusable);

    reusable.Unlink();
    second_external[0] = 777.0;

    Print("reusable after Unlink", reusable);
    std::cout << "second_external[0] = " << second_external[0] << "\n";

    // Проверка ошибок

    Vector invalid_size(-1);
    if (invalid_size.Size() == 0) {
        std::cout << "Ошибка: отрицательный размер обработан\n";
    }

    Vector invalid_link(nullptr, 2);
    Print("invalid_link", invalid_link);

    Vector different_size(2, 1.0);
    double wrong_dot = zeros.Dot(different_size);
    std::cout << "Dot for different sizes = " << wrong_dot << "\n";

    return 0;
}