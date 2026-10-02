#include <cstddef>
#include <iostream>
#include <new>       // std::nothrow

class Vector {
public:
    using size_type = std::size_t;
    // using diff_type = std::ptrdiff_t;

    // Конструктор по умолчанию создаёт пустой владеющий вектор.
    Vector() : data_(nullptr), size_(0), owns_memory_(true) {}

    // Если size отрицательный или память не выделилась, создаётся пустой вектор.
    explicit Vector(int size)
        : data_(nullptr), size_(0), owns_memory_(true) {
        if (size >= 0) {
            size_ = size;
            data_ = Allocate(size_);

            if (data_ == nullptr && size_ != 0) {
                size_ = 0;
            } else {
                Fill(data_, size_, 0.0);
            }
        }
    }

    Vector(int size, double value)
        : data_(nullptr), size_(0), owns_memory_(true) {
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

    // Внешний массив не удаляется этим объектом.
    // При неверных данных получается пустой владеющий вектор.
    Vector(double* external_data, int size)
        : data_(nullptr), size_(0), owns_memory_(true) {
        if (size >= 0 && (size == 0 || external_data != nullptr)) {
            data_ = external_data;
            size_ = size;
            owns_memory_ = false;
        }
    }

    // Копирование владеющего вектора создаёт новый массив.
    // Копирование невладеющего вектора копирует только адрес внешнего массива.
    Vector(const Vector& other)
        : data_(nullptr), size_(0), owns_memory_(other.owns_memory_) {
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

    // Деструктор вызывается автоматически при уничтожении объекта.
    ~Vector() {
        if (owns_memory_) {
            delete[] data_;
        }
    }

    // Присваивание уже существующему объекту: например, a = b.
    Vector& operator=(const Vector& other) {
        if (this == &other) {
            return *this; // Защита от присваивания объекта самому себе.
        }

        if (owns_memory_) {
            delete[] data_;
        }

        data_ = nullptr;
        size_ = 0;
        owns_memory_ = other.owns_memory_;

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

        return *this;
    }

    size_type Size() const {
        return size_;
    }

    bool OwnsMemory() const {
        return owns_memory_;
    }

    // true означает успех, false — неверный индекс.
    bool SetElement(size_type index, double value) {
        if (index >= size_) {
            return false;
        }
        data_[index] = value;
        return true;
    }

    // Значение возвращается через второй параметр.
    bool GetElement(size_type index, double& value) const {
        if (index >= size_) {
            return false;
        }
        value = data_[index];
        return true;
    }

    // Результат записывается в result. Метод не создаёт копий векторов.
    bool Dot(const Vector& other, double& result) const {
        if (size_ != other.size_) {
            return false;
        }

        result = 0.0;
        for (size_type i = 0; i < size_; ++i) {
            result += data_[i] * other.data_[i];
        }
        return true;
    }

    // Подключает внешний массив. При ошибке старое состояние сохраняется.
    bool Link(double* external_data, int size) {
        if (size < 0 || (size != 0 && external_data == nullptr)) {
            return false;
        }

        if (owns_memory_) {
            delete[] data_;
        }

        data_ = external_data;
        size_ = size;
        owns_memory_ = false;
        return true;
    }

    // Создаёт собственную копию внешнего массива.
    bool Unlink() {
        if (owns_memory_) {
            return true; // Уже владеем памятью, делать ничего не нужно.
        }

        double* new_data = Allocate(size_);
        if (new_data == nullptr && size_ != 0) {
            return false;
        }

        Copy(new_data, data_, size_);
        data_ = new_data;
        owns_memory_ = true;
        return true;
    }

private:
    double* data_;
    size_type size_;
    bool owns_memory_;

    static double* Allocate(size_type size) {
        if (size == 0) {
            return nullptr;
        }
        return new double[size];
    }

    static void Fill(double* data, size_type size, double value) {
        for (size_type i = 0; i < size; ++i) {
            data[i] = value;
        }
    }

    static void Copy(double* destination, const double* source, size_type size) {
        for (size_type i = 0; i < size; ++i) {
            destination[i] = source[i];
        }
    }
};

void Print(const char* name, const Vector& vector) {
    std::cout << name << " (size=" << vector.Size()
              << ", owns=" << std::boolalpha << vector.OwnsMemory() << "): [";

    for (Vector::size_type i = 0; i < vector.Size(); ++i) {
        double value = 0.0;
        vector.GetElement(i, value);
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

    // Для владеющего вектора создаётся независимая копия.
    Vector owning_copy(zeros);
    zeros.SetElement(0, 100.0);
    Print("zeros", zeros);
    Print("owning_copy", owning_copy);

    double external[] = {10.0, 20.0, 30.0};
    Vector linked(external, 3);
    Vector linked_copy(linked);
    linked_copy.SetElement(1, 200.0);
    Print("linked", linked);
    std::cout << "external[1] = " << external[1] << "\n";

    Vector left(3, 2.0);
    Vector right(3, 4.0);
    double dot_result = 0.0;
    if (left.Dot(right, dot_result)) {
        std::cout << "Dot = " << dot_result << "\n";
    }

    double second_external[] = {-1.0, -2.0, -3.0};
    Vector reusable(2, 8.0);
    if (reusable.Link(second_external, 3)) {
        reusable.SetElement(0, 123.0);
    }
    reusable.Unlink();
    second_external[0] = 777.0;
    Print("reusable after Unlink", reusable);

    // Примеры обработки ошибок без throw/catch.
    double value = 0.0;
    if (!zeros.GetElement(100, value)) {
        std::cout << "Ошибка: индекс находится за границами вектора\n";
    }

    Vector invalid_size(-1);
    if (invalid_size.Size() == 0) {
        std::cout << "Ошибка: отрицательный размер\n";
    }

    Vector invalid_link;
    if (!invalid_link.Link(nullptr, 2)) {
        std::cout << "Ошибка: nullptr нельзя использовать для непустого массива\n";
    }

    Vector different_size(2, 1.0);
    if (!zeros.Dot(different_size, dot_result)) {
        std::cout << "Ошибка: размеры векторов различаются\n";
    }

    return 0;
}
