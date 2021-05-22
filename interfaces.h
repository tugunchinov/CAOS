#include <string>

class AbstractClass
{
    friend class ClassLoader;
public:
    explicit AbstractClass();
    ~AbstractClass();
protected:
    void* newInstanceWithSize(size_t sizeofClass);
    struct ClassImpl* pImpl;
};

template <class T>
class Class
        : public AbstractClass
{
public:
    T* newInstance()
    {
        size_t classSize = sizeof(T);
        void* rawPtr = newInstanceWithSize(classSize);
        return reinterpret_cast<T*>(rawPtr);
    }
};

enum class ClassLoaderError {
    NoError = 0,
    FileNotFound,
    LibraryLoadError,
    NoClassInLibrary
};


class ClassLoader
{
public:
    explicit ClassLoader();
    AbstractClass* loadClass(const std::string &fullyQualifiedName);
    ClassLoaderError lastError() const;
    ~ClassLoader();
private:
    struct ClassLoaderImpl* pImpl;
};
