#include <iostream>
#include <new>
#include <stdexcept>
using namespace std;

class Vector {
  private:
    double *data_;
    int size_;
    bool owns_memory_;

    static void CheckSize(int size) {
        if (size < 0) {
            throw invalid_argument("Vector size cannot be negative");
        }
    }

    static void CheckExternalBuffer(const double *external_ptr, int size) {
        CheckSize(size);

        if (size > 0 && external_ptr == nullptr) {
            throw invalid_argument("Null external pointer for a non-empty vector");
        }
    }

    void CheckIndex(int index) const {
        if (index < 0 || index >= size_) {
            throw out_of_range("Vector index out of bounds");
        }
    }

    static double *Allocate(int size) noexcept {
        if (size == 0) {
            return nullptr;
        }

        return new double[size];
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

  public:
    // Пустой владеющий вектор.
    Vector() noexcept : data_(nullptr), size_(0), owns_memory_(true) {}

    // Владеющий вектор заданного размера, заполненный нулями.
    explicit Vector(int size) : Vector(size, 0.0) {}

    // Владеющий вектор заданного размера, заполненный value.
    Vector(int size, double value) : data_(nullptr), size_(0), owns_memory_(true) {
        CheckSize(size);

        size_ = size;
        data_ = Allocate(size_);
        Fill(data_, size_, value);
    }

    // Невладеющий вектор, работающий с внешним буфером.
    Vector(double *external_ptr, int size) : data_(nullptr), size_(0), owns_memory_(false) {
        CheckExternalBuffer(external_ptr, size);

        data_ = external_ptr;
        size_ = size;
    }

    // Копирование сохраняет режим владения.
    Vector(const Vector &other)
        : data_(nullptr), size_(other.size_), owns_memory_(other.owns_memory_) {

        if (owns_memory_) {
            data_ = Allocate(size_);
            Copy(data_, other.data_, size_);

        } else {
            data_ = other.data_;
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
        CheckIndex(index);
        data_[index] = value;
        return true;
    }

    double GetElement(int index) const {
        CheckIndex(index);
        return data_[index];
    }

    double Dot(const Vector &other) const {
        if (size_ != other.size_) {
            throw invalid_argument("Dot product requires equal vector sizes");
        }

        double result = 0.0;

        for (int i = 0; i < size_; ++i) {
            result += data_[i] * other.data_[i];
        }

        return result;
    }

    // Переключение в невладеющий режим.
    void Link(double *external_ptr, int size) {
        CheckExternalBuffer(external_ptr, size);

        if (owns_memory_) {
            delete[] data_;
        }

        data_ = external_ptr;
        size_ = size;
        owns_memory_ = false;
    }

    // Создание собственной копии внешнего буфера.
    void Unlink() {
        if (owns_memory_) {
            return;
        }

        double *new_data = Allocate(size_);
        Copy(new_data, data_, size_);

        data_ = new_data;
        owns_memory_ = true;
    }
};

void Print(const char *name, const Vector &vector) {
    cout << name << " (size=" << vector.Size() << ", owns=" << boolalpha << vector.OwnsMemory()
         << "): [";

    for (int i = 0; i < vector.Size(); ++i) {
        if (i != 0) {
            cout << ", ";
        }

        cout << vector.GetElement(i);
    }

    cout << "]\n";
}

int main() {
    // Разные способы создания.
    Vector empty;
    Vector zeros(3);
    Vector filled(4, 2.5);

    zeros.SetElement(0, 1.0);
    zeros.SetElement(1, 2.0);
    zeros.SetElement(2, 3.0);

    Print("empty", empty);
    Print("zeros", zeros);
    Print("filled", filled);

    // Копирование владеющего вектора: независимая копия.
    Vector owning_copy(zeros);
    zeros.SetElement(0, 100.0);

    Print("zeros", zeros);
    Print("owning_copy", owning_copy);

    // Внешний буфер и копирование невладеющего вектора.
    double external[] = {10.0, 20.0, 30.0};

    Vector linked(external, 3);
    Vector linked_copy(linked);

    linked_copy.SetElement(1, 200.0);

    Print("linked", linked);
    Print("linked_copy", linked_copy);
    cout << "external[1] = " << external[1] << "\n";

    // Скалярное произведение без создания копий.
    Vector left(3, 2.0);
    Vector right(3, 4.0);

    cout << "Dot = " << left.Dot(right) << "\n";

    // Link и Unlink.
    double second_external[] = {-1.0, -2.0, -3.0};

    Vector reusable(2, 8.0);
    Print("reusable before Link", reusable);

    reusable.Link(second_external, 3);
    reusable.SetElement(0, 123.0);
    Print("reusable after Link", reusable);
    cout << "second_external[0] = " << second_external[0] << "\n";

    reusable.Unlink();
    second_external[0] = 777.0;

    Print("reusable after Unlink", reusable);
    cout << "second_external[0] = " << second_external[0] << "\n";

    // Обработка некорректных входных данных.
    try {
        Vector invalid_size(-1);
    } catch (const invalid_argument &error) {
        cout << "Invalid size: " << error.what() << "\n";
    }

    try {
        Vector invalid_link(nullptr, 2);
    } catch (const invalid_argument &error) {
        cout << "Invalid external buffer: " << error.what() << "\n";
    }

    try {
        zeros.GetElement(-1);
    } catch (const out_of_range &error) {
        cout << "Invalid index: " << error.what() << "\n";
    }

    try {
        Vector different_size(2, 1.0);
        zeros.Dot(different_size);
    } catch (const invalid_argument &error) {
        cout << "Invalid dot product: " << error.what() << "\n";
    }

    return 0;
}
