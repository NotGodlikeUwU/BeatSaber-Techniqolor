#pragma once
#include "beatsaber-hook/shared/utils/typedefs.h"
#include <memory>
// SafePtrUnity roots a managed Unity object and also checks its native lifetime.
// Shared ownership supplies assignment for STL containers (SafePtrUnity lacks it).
template<typename T> class UnityRef {
    std::shared_ptr<SafePtrUnity<T>> handle;
public:
    UnityRef()=default;
    UnityRef(T* object) { if(object) handle=std::make_shared<SafePtrUnity<T>>(object); }
    UnityRef& operator=(T* object) { handle=object ? std::make_shared<SafePtrUnity<T>>(object) : nullptr; return *this; }
    explicit operator bool() const { return handle && static_cast<bool>(*handle); }
    T* ptr() const { return *this ? handle->ptr() : nullptr; }
    T* operator->() const { return ptr(); }
};
