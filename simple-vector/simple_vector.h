#pragma once

#include <iostream>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <iterator>
#include <algorithm>
 
#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve) {
        m_capacity_to_reserve = capacity_to_reserve;
    }

    size_t GetReserve() noexcept {
        return m_capacity_to_reserve;
    }

private:
    size_t m_capacity_to_reserve = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        ArrayPtr<Type> tmp_vector(size);
        std::generate(tmp_vector.Get(), tmp_vector.Get()+size, [] () mutable { return std::move(Type()); });
        m_vector.swap(tmp_vector);
        m_size = size;
        m_capacity = size;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) {
        ArrayPtr<Type> tmp_vector(size);
        std::fill(tmp_vector.Get(), tmp_vector.Get()+size, std::move(value));
        m_vector.swap(tmp_vector);
        m_size = size;
        m_capacity = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        auto size = init.size();
        ArrayPtr<Type> tmp_vector(size);
        std::copy(init.begin(), init.end(), tmp_vector.Get());
        m_vector.swap(tmp_vector);
        m_size = size;
        m_capacity = size;
    }

    // Создаёт вектор с резервом места
    explicit SimpleVector(ReserveProxyObj obj) {
        size_t capacity = obj.GetReserve();
        ArrayPtr<Type> tmp_vector(capacity);
        std::fill(tmp_vector.Get(), tmp_vector.Get()+capacity, Type());
        m_vector.swap(tmp_vector);
        m_size = 0;
        m_capacity = capacity;
    }

    // Создаёт вектор как копию вектора other
    SimpleVector(const SimpleVector& other) {
        ArrayPtr<Type> tmp_vector(other.GetCapacity());
        std::copy(other.begin(), other.end(), tmp_vector.Get());
        m_vector.swap(tmp_vector);
        m_size = other.GetSize();
        m_capacity = other.GetCapacity();
    }

    // конструктор с move семантикой
    SimpleVector(SimpleVector&& other) noexcept {
        m_vector = std::exchange(other.m_vector, m_vector);
        m_size = std::exchange(other.m_size, m_size);
        m_capacity = std::exchange(other.m_capacity, m_capacity);
    }

    // оператор присваивания
    SimpleVector& operator=(const SimpleVector& rhs) {
        ArrayPtr<Type> tmp_vector(rhs.GetCapacity());
        std::copy(rhs.begin(), rhs.end(), tmp_vector.Get());
        m_vector.swap(tmp_vector);
        m_size = rhs.GetSize();
        m_capacity = rhs.GetCapacity();
        return *this;
    }

    // перемещающий оператор присваивания
    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        m_vector = std::exchange(rhs.m_vector, m_vector);
        m_size = std::exchange(rhs.m_size, m_size);
        m_capacity = std::exchange(rhs.m_capacity, m_capacity);
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return m_size;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return m_capacity;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (0 == m_size);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < m_size);
        return *(begin()+index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < m_size);
        return *(begin()+index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if(index >= GetSize()) {
            using namespace std::string_literals; 
            throw std::out_of_range("out of range"s);
        }
        return *(begin()+index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if(index >= GetSize()) {
            using namespace std::string_literals; 
            throw std::out_of_range("out of range"s);
        }
        return *(begin()+index);
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        m_size = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if(new_size <= m_size) {
            // новый размер меньше или равен текущему
            m_size = new_size;
            return;
        }
        // ? новый размер не превышает вместимости вектора : новый размер превышает вместимость вектора
        size_t capacity = new_size <= m_capacity ? m_capacity : std::max(new_size, 2*m_capacity);
        SimpleVector tmp_vector(capacity);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), tmp_vector.begin());
        swap(std::move(tmp_vector));
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        if(!IsEmpty()) {
            return Iterator{m_vector.Get()};
        }
        return Iterator{nullptr};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        if(!IsEmpty()) {
            return Iterator{m_vector.Get()+m_size};
        }
        return Iterator{nullptr};
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        if(!IsEmpty()) {
            return ConstIterator{m_vector.Get()};
        }
        return ConstIterator{nullptr};
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        if(!IsEmpty()) {
            return ConstIterator{m_vector.Get()+m_size};
        }
        return ConstIterator{nullptr};
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return end();
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        // прежний размер
        size_t prev_size = m_size;
        // увеличиваем вектор
        Resize(m_size+1);
        // ставим item в позицию перед end
        *(std::next(begin(), prev_size)) = item;
        // ставим корректный размер
        m_size = prev_size + 1;
    }

    // Перемещает элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(Type&& item) {
        // прежний размер
        size_t prev_size = m_size;
        // увеличиваем вектор
        Resize(m_size+1);
        // перемещаем item в позицию перед end
        *(std::next(begin(), prev_size)) = std::exchange(item, Type());
        // ставим корректный размер
        m_size = prev_size + 1;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        // прежний размер
        size_t prev_size = m_size;
        // расстояние от begin до pos
        auto dist = std::distance(begin(), const_cast<Iterator>(pos));
        // увеличиваем вектор на один
        Resize(m_size+1);
        // итератор на нужную позицию
        Iterator insert_pos = std::next(begin(), dist);
        // сдвигаем элементы
        std::copy_backward(insert_pos, static_cast<Iterator>(std::next(begin(), prev_size)), static_cast<Iterator>(std::next(begin(), prev_size+1)));
        // вставляем значение
        *insert_pos = value;
        // ставим корректный размер
        m_size = prev_size + 1;
        return insert_pos;
    }

    // Перемещает значение value в позицию pos.
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        // прежний размер
        size_t prev_size = m_size;
        // расстояние от begin до pos
        auto dist = std::distance(begin(), const_cast<Iterator>(pos));
        // увеличиваем вектор на один
        Resize(m_size+1);
        // итератор на нужную позицию
        Iterator insert_pos = std::next(begin(), dist);
        // сдвигаем элементы
        std::move_backward(insert_pos, static_cast<Iterator>(std::next(begin(), prev_size)), static_cast<Iterator>(std::next(begin(), prev_size+1)));
        // переносим значение
        *insert_pos = std::exchange(value, Type());
        // ставим корректный размер
        m_size = prev_size + 1;
        return insert_pos;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if(0 != m_size) {
            --m_size;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        Iterator pos_copy = const_cast<Iterator>(pos);
        for(Iterator it = std::next(const_cast<Iterator>(pos), 1); it != end(); it++) {
            std::swap(*pos_copy, *it);
            std::advance(pos_copy, 1);
        }
        --m_size;
        if(0 == m_size)
            return Iterator{nullptr};
        else
            return const_cast<Iterator>(pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        m_vector.swap(other.m_vector);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    // Обменивает значение перемещением с другим вектором
    void swap(SimpleVector&& other) noexcept {
        m_vector.swap(std::move(other.m_vector));
        other.m_size = std::exchange(m_size, other.m_size);
        other.m_capacity = std::exchange(m_capacity, other.m_capacity);
    }

    // Резервируем место
    void Reserve(size_t new_capacity) {
        if(m_capacity < new_capacity) {
            ArrayPtr<Type> tmp_vector(new_capacity);
            std::copy(begin(), end(), tmp_vector.Get());
            m_vector.swap(tmp_vector);
            m_capacity = new_capacity;
        }
    }

private:
    ArrayPtr<Type> m_vector;
    size_t   m_size = 0;
    size_t   m_capacity = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
