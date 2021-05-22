#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>

extern "C" {
#include <dlfcn.h>
}
  
#include "interfaces.h"

typedef void (*constructor_t)(void*);

struct ClassImpl {
  void* lib = nullptr;
  std::string class_name;
};

AbstractClass::AbstractClass(): pImpl(new ClassImpl()) {}
AbstractClass::~AbstractClass() {
  if (pImpl->lib) {
    dlclose(pImpl->lib);
  }
  delete pImpl;
}

std::string CreateSymbolicName(const std::string& class_name) {
  size_t pos = 0;
  size_t new_pos = 0;
  std::string sym_name = "_ZN";
  while ((new_pos = class_name.find("::", pos)) != std::string::npos) {
    sym_name += std::to_string(new_pos - pos);
    sym_name += class_name.substr(pos, new_pos - pos);
    pos = new_pos + 2;
  }
  sym_name += std::to_string(class_name.size() - pos);
  sym_name += class_name.substr(pos);
  sym_name += "C1Ev";
  return sym_name;
}

void* AbstractClass::newInstanceWithSize(size_t size_of_class) {
  std::string sym_name = CreateSymbolicName(pImpl->class_name);
  constructor_t constructor =
    reinterpret_cast<constructor_t>(dlsym(pImpl->lib, sym_name.c_str()));
  void* obj = new char[size_of_class];
  constructor(obj);
  return obj;
} 
 
struct AbstractClassList {
  AbstractClass* abstract_class = nullptr;
  AbstractClassList* next = nullptr;
};

struct ClassLoaderImpl {
  AbstractClassList* head = nullptr;
  AbstractClassList* cur = nullptr;
};

ClassLoader::ClassLoader(): pImpl(new ClassLoaderImpl()) {}
ClassLoader::~ClassLoader() {
  while (!pImpl->head) {
    pImpl->cur = pImpl->head;
    pImpl->head = pImpl->head->next;
    delete pImpl->cur;
  }
  delete pImpl;
}

std::string parsePath(const std::string& fully_qualified_name) {
  std::string result = fully_qualified_name;
  std::replace(result.begin(), result.end(), ':', '/');
  return std::string(std::getenv("CLASSPATH")) + "/" + result + ".so";
}


AbstractClass* ClassLoader::loadClass(const std::string& fully_qualified_name) {
  auto path = parsePath(fully_qualified_name);
  void* lib = dlopen(path.c_str(), RTLD_NOW|RTLD_GLOBAL);
  if (!lib) {
    return nullptr;
  }
    
  if (!pImpl->head) {
    pImpl->head = new AbstractClassList();
    pImpl->cur = pImpl->head;
  } else {
    pImpl->cur->next = new AbstractClassList();
    pImpl->cur = pImpl->cur->next;
  } 
  pImpl->cur->abstract_class = new AbstractClass();
  pImpl->cur->abstract_class->pImpl->lib = lib;
  pImpl->cur->abstract_class->pImpl->class_name = fully_qualified_name;
    
  return pImpl->cur->abstract_class;
}
