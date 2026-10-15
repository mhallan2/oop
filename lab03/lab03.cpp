#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>
using namespace std;

// Size
struct Size {
    int rows;
    int cols;
};

bool operator==(const Size &first, const Size &second) {
    return first.rows == second.rows && first.cols == second.cols;
}

bool operator!=(const Size &first, const Size &second) { return !(first == second); }

ostream &operator<<(ostream &out, const Size &size) {
    out << size.rows << "x" << size.cols;
    return out;
}

// Matrix
class Matrix {

  private:
    int rows_;
    int cols_;
    double *data_;
    bool owns_;

    void CheckCols(int col) const {
        if (col < 1 || col > cols_) {
            throw out_of_range("Column index out of bounds");
        }
    }

    void CheckRows(int row) const {
        if (row < 1 || row > rows_) {
            throw out_of_range("Row index out of bounds");
        }
    }

    static void copyData(const double *source, double *destination, int size) {
        for (int i = 0; i < size; ++i) {
            destination[i] = source[i];
        }
    }

    static void FillData(double *data, int size, double value) {
        for (int i = 0; i < size; ++i) {
            data[i] = value;
        }
    }

  public:
    // Row class for non-const Matrix
    class Row {

      private:
        Matrix &matrix_;
        int row_;

      public:
        Row(Matrix &matrix, int row) : matrix_(matrix), row_(row) { matrix_.CheckRows(row_); }

        double &operator[](int col) {
            matrix_.CheckCols(col);

            return matrix_.data_[(row_ - 1) * matrix_.cols_ + (col - 1)];
        }

        const double &operator[](int col) const {
            matrix_.CheckCols(col);

            return matrix_.data_[(row_ - 1) * matrix_.cols_ + (col - 1)];
        }

        int Size() const { return matrix_.cols_; }

        operator Matrix() const;
    };

    // Row class for const Matrix
    class ConstRow {

      private:
        const Matrix &matrix_;
        int row_;

      public:
        ConstRow(const Matrix &matrix, int row) : matrix_(matrix), row_(row) {
            matrix_.CheckRows(row_);
        }

        const double &operator[](int col) const {
            matrix_.CheckCols(col);

            return matrix_.data_[(row_ - 1) * matrix_.cols_ + (col - 1)];
        }

        int Size() const { return matrix_.cols_; }

        operator Matrix() const;
    };

    Row operator[](int row) { return Row(*this, row); }

    ConstRow operator[](int row) const { return ConstRow(*this, row); }

    // Constructors
    Matrix() : rows_(0), cols_(0), data_(nullptr), owns_(true) {}

    explicit Matrix(double value) : rows_(1), cols_(1), data_(new double[1]{value}), owns_(true) {}

    Matrix(int rows, int cols, double value) : rows_(0), cols_(0), data_(nullptr), owns_(true) {

        if (rows <= 0 || cols <= 0)
            return;

        rows_ = rows;
        cols_ = cols;

        data_ = new double[rows_ * cols_];
        FillData(data_, rows_ * cols_, value);
    }

    Matrix(int rows, int cols) : Matrix(rows, cols, 0.0) {}

    Matrix(int rows, int cols, double *data)
        : rows_(rows), cols_(cols), data_(data), owns_(false) {}

    // Rule of Five
    // 1
    ~Matrix() {

        if (owns_) {
            delete[] data_;
        }
    }

    // 2
    Matrix(const Matrix &other)
        : rows_(other.rows_), cols_(other.cols_), data_(nullptr), owns_(true) {

        if (other.rows_ == 0 || other.cols_ == 0)
            return;

        data_ = new double[rows_ * cols_];
        copyData(other.data_, data_, rows_ * cols_);
    }

    // 3
    Matrix(Matrix &&other) noexcept
        : rows_(other.rows_), cols_(other.cols_), data_(other.data_), owns_(other.owns_) {

        other.rows_ = other.cols_ = 0;
        other.data_ = nullptr;
        other.owns_ = true;
    }

    // 4
    Matrix &operator=(const Matrix &other) {

        if (this == &other) {
            return *this;
        }

        if (owns_) {
            delete[] data_;
        }

        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = nullptr;
        owns_ = true;

        if (rows_ != 0 && cols_ != 0) {
            data_ = new double[rows_ * cols_];
            copyData(other.data_, data_, rows_ * cols_);
        }

        return *this;
    }

    // 5
    Matrix &operator=(Matrix &&other) noexcept {

        if (this == &other) {
            return *this;
        }

        if (owns_) {
            delete[] data_;
        }

        data_ = other.data_;
        rows_ = other.rows_;
        cols_ = other.cols_;
        owns_ = other.owns_;

        other.data_ = nullptr;
        other.rows_ = other.cols_ = 0;
        other.owns_ = true;

        return *this;
    }

    Matrix T() const {

        Matrix transposed(cols_, rows_);

        for (int i = 1; i <= rows_; ++i) {
            for (int j = 1; j <= cols_; ++j) {
                transposed.SetElement(j, i, GetElement(i, j));
            }
        }

        return transposed;
    }

    // ()
    double &operator()(int row, int col) {

        CheckRows(row);
        CheckCols(col);

        return data_[(row - 1) * cols_ + (col - 1)];
    }

    const double &operator()(int row, int col) const {

        CheckRows(row);
        CheckCols(col);

        return data_[(row - 1) * cols_ + (col - 1)];
    }

    // +=
    Matrix &operator+=(const Matrix &other) {
        if (GetSize() != other.GetSize()) {
            return *this;
        }

        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] += other.data_[i];
        }

        return *this;
    }

    // -=
    Matrix &operator-=(const Matrix &other) {
        if (GetSize() != other.GetSize()) {
            return *this;
        }

        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] -= other.data_[i];
        }

        return *this;
    }

    // *=
    Matrix &operator*=(const Matrix &other) {
        int m = rows_;
        int n = cols_;
        int k = other.cols_;

        if (n != other.rows_) {
            throw std::invalid_argument("Matrix dimensions mismatch for *=");
        }

        Matrix tmp(m, k);

        for (int i = 1; i <= m; ++i) {
            for (int j = 1; j <= k; ++j) {
                double c_ij = 0.0;
                for (int r = 1; r <= n; ++r) {
                    c_ij += (*this)(i, r) * other(r, j);
                }
                tmp.SetElement(i, j, c_ij);
            }
        }

        return *this = move(tmp);
    }

    // *= scalar
    Matrix &operator*=(const double scalar) {
        for (int i = 0; i < rows_ * cols_; ++i) {
            data_[i] *= scalar;
        }
        return *this;
    }

    Matrix LPad(int rows, int cols, double value) const {
        // if (rows <= 0 || cols <= 0)
        //     throw std::invalid_argument("bad size");
        int minRows = min(rows, rows_);
        int minCols = min(cols, cols_);

        Matrix res(rows, cols, value);

        for (int i = 0; i < minRows; ++i) {
            copyData(data_ + i * cols_, res.data_ + i * cols, minCols);
        }

        return res;
    }

    Matrix RPad(int rows, int cols, double value) const {
        if (rows < 0 || cols < 0) {
            throw std::invalid_argument("Bad matrix size");
        }

        Matrix result(rows, cols, value);

        int copiedRows = min(rows, rows_);
        int copiedCols = min(cols, cols_);

        // Если уменьшаем матрицу, начинаем читать справа/снизу.
        int sourceRow = rows_ - copiedRows;
        int sourceCol = cols_ - copiedCols;

        // Если увеличиваем матрицу, записываем вправо/вниз.
        int destinationRow = rows - copiedRows;
        int destinationCol = cols - copiedCols;

        for (int i = 0; i < copiedRows; ++i) {
            copyData(data_ + (sourceRow + i) * cols_ + sourceCol,

                     result.data_ + (destinationRow + i) * cols + destinationCol,

                     copiedCols);
        }

        return result;
    }

    Matrix LPad(int rows, int cols) const { return LPad(rows, cols, 0.0); }
    Matrix RPad(int rows, int cols) const { return RPad(rows, cols, 0.0); }

    // Matrix &LPad(int rows, int cols, double value) {
    //     *this = static_cast<const Matrix &>(*this).LPad(rows, cols, value);
    //     return *this;
    // }

    // Matrix &RPad(int rows, int cols, double value) {
    //     *this = static_cast<const Matrix &>(*this).RPad(rows, cols, value);
    //     return *this;
    // }

    // ++
    Matrix &operator++() {
        *this = LPad(rows_ + 1, cols_ + 1, 0.0);
        return *this;
    }

    // --
    Matrix &operator--() {
        *this = LPad(rows_ - 1, cols_ - 1, 0.0);
        return *this;
    }

    // ++
    Matrix operator++(int) {
        Matrix old = *this;
        *this = RPad(rows_ + 1, cols_ + 1, 0.0);
        return old;
    }

    // --
    Matrix operator--(int) {
        Matrix old = *this;
        *this = RPad(rows_ - 1, cols_ - 1, 0.0);
        return old;
    }

    // Getters
    Size GetSize() const { return {rows_, cols_}; }

    int Rows() const { return rows_; }

    int Cols() const { return cols_; }

    bool OwnsMemory() const { return owns_; }

    double GetElement(int i, int j) const {
        if (i < 1 || j < 1 || i > rows_ || j > cols_) {
            throw out_of_range("Index out of bounds");
        }

        return data_[(i - 1) * cols_ + (j - 1)];
    }

    // Setters
    void SetElement(int i, int j, double value) {
        if (i < 1 || j < 1 || i > rows_ || j > cols_) {
            throw out_of_range("Index out of bounds");
        }

        data_[(i - 1) * cols_ + (j - 1)] = value;
    }

    // Friend functions
    friend ostream &operator<<(ostream &os, const Matrix &other);
    friend istream &operator>>(istream &is, Matrix &other);
    friend bool operator==(const Matrix &first, const Matrix &second);
    friend bool operator!=(const Matrix &first, const Matrix &second);
};

// Matrix conversion
Matrix::Row::operator Matrix() const {
    Matrix result(1, matrix_.cols_);

    for (int col = 1; col <= matrix_.cols_; ++col) {
        result.SetElement(1, col, (*this)[col]);
    }

    return result;
}

Matrix::ConstRow::operator Matrix() const {
    Matrix result(1, matrix_.cols_);

    for (int col = 1; col <= matrix_.cols_; ++col) {
        result.SetElement(1, col, (*this)[col]);
    }

    return result;
}

// <<
ostream &operator<<(ostream &out, const Matrix &matrix) {
    Size size = matrix.GetSize();
    out << " (size=" << size << ", owns=" << boolalpha << matrix.OwnsMemory() << "):\n";

    out << "[";
    for (int i = 1; i <= matrix.rows_; ++i) {

        out << "[";

        for (int j = 1; j <= matrix.cols_; ++j) {
            if (j != 1) {
                out << ", ";
            }
            out << matrix(i, j);
        }

        out << "]";

        if (i != matrix.rows_) {
            out << ",\n ";
        }
    }

    out << "]\n";
    return out;
}

// >>
istream &operator>>(istream &in, Matrix &matrix) {
    int rows;
    int cols;

    in >> rows >> cols;
    Matrix m(rows, cols);
    for (int i = 1; i <= rows; ++i) {
        for (int j = 1; j <= cols; ++j) {
            double value;
            in >> value;
            m.SetElement(i, j, value);
        }
    }
    matrix = move(m);
    return in;
}

// ==
bool operator==(const Matrix &first, const Matrix &second) {
    if (first.GetSize() != second.GetSize()) {
        return false;
    }

    for (int i = 1; i <= first.rows_; ++i) {
        for (int j = 1; j <= first.cols_; ++j) {
            if (first(i, j) != second(i, j)) {
                return false;
            }
        }
    }

    return true;
}

// !=
bool operator!=(const Matrix &first, const Matrix &second) { return !(first == second); }

// +
Matrix operator+(const Matrix &first, const Matrix &second) {
    Matrix sum = first;
    return sum += second;
}

// *
Matrix operator*(const Matrix &first, const Matrix &second) {
    Matrix res = first;
    return res *= second;
}

// ^
Matrix operator^(const Matrix &matrix, int n) {
    if (n <= 0) {
        throw invalid_argument("Exponent must be non-negative");
    }

    Matrix res = matrix;

    for (int i = 1; i < n; i++) {
        res *= matrix;
    }

    return res;
}

// * scalar
Matrix operator*(const Matrix &matrix, const double scalar) {
    Matrix res = matrix;
    return res *= scalar;
}
Matrix operator*(const double scalar, const Matrix &matrix) { return matrix * scalar; }

// -
Matrix operator-(const Matrix &first, const Matrix &second) {
    Matrix sum = first;
    return sum -= second;
}

int main() {
    // 1. Конструкторы и владение памятью
    cout << "========== 1. constructors ==========" << endl;

    Matrix empty; // 0x0, пустая
    cout << "empty" << empty;

    Matrix one(5.0); // 1x1 explicit
    cout << "one (1x1)" << one;

    Matrix filled(2, 3, 7.0); // 2x3, все 7
    cout << "filled (2x3)" << filled;

    Matrix zeros(2, 3); // 2x3, все 0
    cout << "zeros (2x3)" << zeros;

    // view без владения (owns=false)
    double raw[] = {1, 2, 3, 4, 5, 6};
    Matrix view(2, 3, raw);
    cout << "view " << view;

    // копия — глубокая, owns=true
    Matrix copy = filled;
    copy.SetElement(1, 1, 99.0);
    cout << "copy of filled (element [1][1] = 99)" << copy;
    cout << "filled unchanged" << filled;

    // перемещение — данные переезжают, источник пустеет
    Matrix moved = std::move(copy);
    cout << "moved-from copy" << copy; // 0x0
    cout << "moved-to" << moved;

    // 2. Арифметика
    cout << "========== 2. arithmetic ==========" << endl;

    Matrix a(2, 3, 1.0);
    Matrix b(2, 3, 2.0);
    cout << "a" << a;
    cout << "b" << b;

    cout << "a + b" << (a + b);
    cout << "b - a" << (b - a);

    a += b;
    cout << "a += b" << a;

    Matrix c(3, 2, 1.0);
    cout << "c" << c;
    cout << "b * c" << (b * c);

    cout << "b * 3.0" << (b * 3.0);
    cout << "2.0 * b" << (2.0 * b);

    // 3. LPad / RPad — pad и crop
    cout << "========== 3. LPad / RPad ==========" << endl;

    const Matrix small(1.0); // 1x1 [[1]]
    cout << "small" << small;

    // LPad: рост, контент в левый верхний угол
    cout << "small.LPad(3,3,0) — контент в левый верхний" << small.LPad(3, 3, 0.0);

    // RPad: рост, контент в правый нижний угол
    cout << "small.RPad(3,3,0) — контент в правый нижний" << small.RPad(3, 3, 0.0);

    // crop через LPad: берём левый верхний
    Matrix big = small.LPad(3, 3, 0.0);
    cout << "big" << big;
    cout << "big.LPad(1,1,0) — crop до левого верхнего" << big.LPad(1, 1, 0.0);

    // crop через RPad: берём правый нижний
    Matrix bigR = small.RPad(3, 3, 0.0);
    cout << "bigR" << bigR;
    cout << "bigR.RPad(1,1,0) — crop до правого нижнего" << bigR.RPad(1, 1, 0.0);

    // crop по одной оси, pad по другой
    Matrix rect(3, 4);
    int v = 1;
    for (int i = 1; i <= 3; ++i)
        for (int j = 1; j <= 4; ++j)
            rect.SetElement(i, j, v++);
    cout << "rect (3x4)" << rect;
    cout << "rect.LPad(2,6,0) — crop строки, pad столбцы" << rect.LPad(2, 6, 0.0);

    // 4. Индексация: [] и ()

    cout << "========== 4. indexing ==========" << endl;

    Matrix A(3, 3);
    v = 1;
    for (int i = 1; i <= 3; ++i)
        for (int j = 1; j <= 3; ++j)
            A.SetElement(i, j, v++);
    cout << "A" << A;

    cout << "A[2] — вторая строка (view)" << A[2];        // 1x3
    cout << "A[2][3] = " << A[2][3] << endl;              // 6
    cout << "A(2,3)  = " << A(2, 3) << endl;              // 6
    cout << "A[2][2] через double = " << A[2][2] << endl; // 5

    for (int i = 1; i <= A.Rows(); ++i) {
        cout << "Строка #" << i << A[i];
    }

    A[1][1] = 99.0;
    cout << "A после A[1][1] = 99" << A;
    A(1, 2) = 88.0;
    cout << "A после A(1,2) = 88" << A;

    // 5. Транспонирование

    cout << "========== 5. transpose ==========" << endl;

    Matrix B(2, 3);
    v = 1;
    for (int i = 1; i <= 2; ++i)
        for (int j = 1; j <= 3; ++j)
            B.SetElement(i, j, v++);
    cout << "B (2x3)" << B;
    cout << "B.T() (3x2)" << B.T();

    // 6. Степень

    cout << "========== 6. power ==========" << endl;

    Matrix P(2, 2, 2.0);
    cout << "P" << P;
    cout << "P ^ 1" << (P ^ 1);
    cout << "P ^ 2" << (P ^ 2);
    cout << "P ^ 3" << (P ^ 3);

    // 7. Инкремент / декремент

    cout << "========== 7. ++ / -- ==========" << endl;

    Matrix m(2, 2, 3.0);
    cout << "m" << m;

    cout << "m++ (возвращает старое, m растёт через RPad)" << m++;
    cout << "m после m++" << m;

    cout << "++m (растёт через LPad, возвращает новое)" << ++m;
    cout << "m после ++m" << m;

    cout << "m-- (возвращает старое)" << m--;
    cout << "m после m--" << m;

    cout << "--m (уменьшает через LPad)" << --m;
    cout << "m после --m" << m;

    Matrix test(1, 1, 5.0);
    cout << "test" << test;
    cout << "--test" << --test;

    return 0;
}
