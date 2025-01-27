#include <Eigen/Dense>
#include <thrust/device_vector.h>
#include <thrust/transform.h>
#include <thrust/functional.h>

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


class GPUStorage : public StorageInterface<Eigen::MatrixXf> {
private:
    thrust::device_vector<float>  gpu_vec;
public:
    GPUStorage(Eigen::MatrixXf A) : gpu_vec(A.data(), A.data() + A.size()) {}

    Eigen::MatrixXf  get() const override {
        //return *value;
        return Eigen::MatrixXf();
    }

    void set(Eigen::MatrixXf newValue) override {
        
    }
};

