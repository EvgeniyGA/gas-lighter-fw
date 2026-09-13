#include <cstddef>  // Для std::size_t
#include <new>      // Для std::align_val_t (нужно для C++17 и выравнивания)

// =====================================================================
// Перехват operator new
// =====================================================================
void* operator new(std::size_t size) {
    (void)size;
    __asm volatile("bkpt #0"); // Остановит отладчик (STM32)
    while(1) {}                // Вечный цикл, если запустили без отладчика
    return nullptr;
}

void* operator new[](std::size_t size) {
    (void)size;
    __asm volatile("bkpt #0");
    while(1) {}
    return nullptr;
}

// Версия new с nothrow (иногда используется внутри STL)
void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
    (void)size;
    __asm volatile("bkpt #0");
    while(1) {}
    return nullptr;
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
    (void)size;
    __asm volatile("bkpt #0");
    while(1) {}
    return nullptr;
}

// =====================================================================
// Перехват operator delete (ОБЯЗАТЕЛЬНО с noexcept!)
// =====================================================================
void operator delete(void* ptr) noexcept {
    (void)ptr;
    __asm volatile("bkpt #0");
    while(1) {}
}

void operator delete[](void* ptr) noexcept {
    (void)ptr;
    __asm volatile("bkpt #0");
    while(1) {}
}

void operator delete(void* ptr, std::size_t size) noexcept {
    (void)ptr; (void)size;
    __asm volatile("bkpt #0");
    while(1) {}
}

void operator delete[](void* ptr, std::size_t size) noexcept {
    (void)ptr; (void)size;
    __asm volatile("bkpt #0");
    while(1) {}
}

// Версии delete с nothrow
void operator delete(void* ptr, const std::nothrow_t&) noexcept {
    (void)ptr;
    __asm volatile("bkpt #0");
    while(1) {}
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
    (void)ptr;
    __asm volatile("bkpt #0");
    while(1) {}
}