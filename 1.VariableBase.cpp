#include <iostream>
#include <functional>
#include <map>
#include <list>
#include <vector>

std::map<size_t, std::vector<size_t>> affectionMap;
template <typename T>
class StorageInterface {
public:
    virtual T get() const = 0; // Get the value
    virtual void set(T value) = 0; // Set the value
    virtual ~StorageInterface() = default; // Virtual destructor for polymorphism
};
template <typename T>
class DirectStorage : public StorageInterface<T> {
private:
    std::shared_ptr<T> value; // Directly store the value

public:
    DirectStorage(T initialValue = -1) : value(std::make_shared<T>(initialValue)) {}

    T get() const override {
        return *value;
    }

    void set(T newValue) override {
        *value = (newValue);
    }
};

template <typename T, template <typename> class StorageType>
class Value {
protected:
    StorageType <T> storage;
public:
    Value() : storage() {
        std::cout << "b";
    }
    Value(T initialValue) : storage(initialValue) {
        std::cout << "b";
    }
    Value(const Value& other) : storage(other.storage) {
        std::cout << "b";
    }
    // Getter
    virtual T getValue() {
        return storage.get();
    }
    // Setter
    void setValue(T value) {
        storage.set(value);
    }
};

template <typename T>
class OperatorBase {
public:
    virtual ~OperatorBase() = default;

    // Pure virtual method to evaluate and return type T
    virtual T evaluate() const = 0;
};
template <typename T, template <typename> class Storage>
class Variable;

template <typename T, typename T1, typename T2, template <typename> class S >
class ProductOperator : public OperatorBase<T> {
    std::shared_ptr<Value<T1, S>> lhs;
    std::shared_ptr<Value<T2, S>> rhs;

    // Define the operation, e.g., multiplication
    T operation(const std::shared_ptr<Value<T1, S>> l, const  std::shared_ptr<Value<T2, S>> r) const {
        return l->getValue() * r->getValue(); // Example: multiply lhs and rhs
    }

public:
    ProductOperator(Value<T1, S>& l, Value<T2, S>& r) :
        lhs(std::make_shared<Value<T1, S>>(l)),
        rhs(std::make_shared<Value<T2, S>>(r))
    {
        std::cout << ":a";
    }
    ProductOperator(Variable<T1, S>& l, Value<T2, S>& r) :
        lhs(std::static_pointer_cast<Value<T1, S>>(std::make_shared<Variable<T1, S>>(l))),
        rhs(std::make_shared<Value<T2, S>>(r))
    {
        std::cout << ":a";
    }
    ProductOperator(Variable<T1, S>& l, Variable<T2, S>& r) :
        lhs(std::static_pointer_cast<Value<T1, S>>(std::make_shared<Variable<T1, S>>(l))),
        rhs(std::static_pointer_cast<Value<T2, S>>(std::make_shared<Variable<T2, S>>(r)))
    {
        std::cout << ":a";
    }

    // Implement the evaluate function
    T evaluate() const override {
        return operation(lhs, rhs);
    }
};
template <typename T, template <typename> class StorageType>
class Variable : public Value<T, StorageType> {
    size_t id;
    struct SharedState {
        bool computed;                               // Shared computed flag
        std::shared_ptr<OperatorBase<T>> function; // Shared operator

        SharedState(bool computed, std::shared_ptr<OperatorBase<T>> func = nullptr)
            : computed(computed), function(func) {
        }
    };
    std::shared_ptr<SharedState> state;
public:
    Variable(T v) :Value<T, StorageType>(v), state(std::make_shared<SharedState>(SharedState(true)))
    {
    }
    //static std::map<size_t, std::list<Variable>> myMap;
    template <typename T1, typename T2>
    Variable(const ProductOperator<T, T1, T2, StorageType>& op)
        : Value<T, StorageType>(-1), state(std::make_shared<SharedState>(SharedState(false, std::make_shared<ProductOperator<T, T1, T2, StorageType>>(op)))) {

    }

    template <typename T1, typename T2, template  <typename> class S >
    friend Variable<decltype(T1()* T2()), S> operator *(Variable<T1, S>& lhs, Variable<T2, S>& rhs) {
        using ReturnType = decltype(T1()* T2());
        return Variable<ReturnType, S>(ProductOperator<ReturnType, T1, T2, S>(lhs, rhs));
    }
    T getValue() override {
        if (!state->computed)
        {
            this->setValue(state->function->evaluate());
            state->computed = true;
        }
        return Value<T, StorageType >::getValue();
    }
};

int main()
{
    Variable<int, DirectStorage>  a(2);
    Variable<int, DirectStorage>  b(3);
    auto  c = a * b;
    auto d = c * b;
    std::cout << c.getValue();
    std::cout << d.getValue();
    Variable<int, DirectStorage>  a(2);
}