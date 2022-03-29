#include <iostream>
#include <cassert>
#include <cstdlib>
#include <utility>

template <typename Type>
class ArrayPtr {
public:
    // Инициализирует ArrayPtr нулевым указателем
    ArrayPtr() = default;

    // Создаёт в куче массив из size элементов типа Type.
    // Если size == 0, поле raw_ptr_ должно быть равно nullptr
    explicit ArrayPtr(size_t size) {
        assert(nullptr == raw_ptr_);
        if(0 != size) {
            raw_ptr_ = new Type[size]{};
        }
    }

    // Конструктор из сырого указателя, хранящего адрес массива в куче либо nullptr
    explicit ArrayPtr(Type* raw_ptr) noexcept {
        assert(nullptr == raw_ptr_);
        raw_ptr_ = raw_ptr;
    }

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    // копирующий конструктор  
    ArrayPtr(const ArrayPtr& other) {
        // создаем вектор
        ArrayPtr<Type> tmp_vector(other.Get());
        // меняем местами
        swap(tmp_vector);
    }

    // оператор присваивания
    ArrayPtr& operator=(const ArrayPtr& other) {
        // создаем вектор
        ArrayPtr<Type> tmp_vector(other.Get());
        // меняем местами
        swap(tmp_vector);
        return *this;
    }

    // перемещающий конструктор
    ArrayPtr(ArrayPtr&& other) noexcept {
        // перемещаем
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);
    }

    // перемещающий оператор присваивания
    ArrayPtr& operator=(ArrayPtr&& other) noexcept {
        // перемещаем
        raw_ptr_ = std::exchange(other.raw_ptr_, nullptr);
        return *this;
    }

    // Прекращает владением массивом в памяти, возвращает значение адреса массива
    // После вызова метода указатель на массив должен обнулиться
    [[nodiscard]] Type* Release() noexcept {
        Type* p = raw_ptr_;
        raw_ptr_ = nullptr;
        return p;
    }

    // Возвращает ссылку на элемент массива с индексом index
    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_+index);
    }

    // Возвращает константную ссылку на элемент массива с индексом index
    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_+index);
    }

    // Возвращает true, если указатель ненулевой, и false в противном случае
    explicit operator bool() const {
        return nullptr != raw_ptr_;
    }

    // Возвращает значение сырого указателя, хранящего адрес начала массива
    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // Обменивается значениям указателя на массив с объектом other
    void swap(ArrayPtr& other) noexcept {
        std::swap(raw_ptr_, other.raw_ptr_);
    }

    void swap(ArrayPtr&& other) noexcept {
        raw_ptr_ = std::exchange(other.raw_ptr_, raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};
