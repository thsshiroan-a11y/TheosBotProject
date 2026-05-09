#pragma once
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <dlfcn.h>
#import <mach/mach.h>
#import <sys/mman.h>
#import <unistd.h>

// ===== DYNAMIC LIBRARY LOADING =====

typedef void (*dynamic_func_t)(void);

// Handle for loaded library
static void *g_dynamic_handle = NULL;

// Load library dynamically with obfuscation
static inline void* load_library_obfuscated(const char *libraryName) {
    if (!libraryName) return NULL;
    
    // Obfuscated paths
    const char *paths[] = {
        "/usr/lib/lib",
        "/Library/MobileSubstrate/DynamicLibraries/",
        "/var/mobile/Library/DynamicLibraries/"
    };
    
    for (int i = 0; i < 3; i++) {
        char fullPath[256];
        snprintf(fullPath, sizeof(fullPath), "%s%s.dylib", paths[i], libraryName);
        
        void *handle = dlopen(fullPath, RTLD_NOW | RTLD_LOCAL);
        if (handle) {
            g_dynamic_handle = handle;
            return handle;
        }
    }
    
    // Try direct
    return dlopen(libraryName, RTLD_NOW | RTLD_LOCAL);
}

// Unload library
static inline void unload_library(void *handle) {
    if (handle && handle != g_dynamic_handle) {
        dlclose(handle);
    }
}

// Get function address dynamically
static inline dynamic_func_t get_function_obfuscated(void *handle, const char *funcName) {
    if (!handle || !funcName) return NULL;
    return (dynamic_func_t)dlsym(handle, funcName);
}

// ===== RUNTIME CODE GENERATION =====

// Allocate executable memory
static inline void* alloc_executable_memory(size_t size) {
    void *memory = mmap(NULL, size, 
                      PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return memory;
}

// Free executable memory
static inline void free_executable_memory(void *memory, size_t size) {
    if (memory) {
        munmap(memory, size);
    }
}

// ===== FUNCTION POINTER TABLE =====

// Function pointer table for indirect calls
typedef struct {
    const char *name;
    dynamic_func_t func;
} func_table_entry_t;

static func_table_entry_t g_func_table[32];
static int g_func_count = 0;

// Register function
static inline void register_function(const char *name, dynamic_func_t func) {
    if (g_func_count < 32 && name && func) {
        g_func_table[g_func_count].name = name;
        g_func_table[g_func_count].func = func;
        g_func_count++;
    }
}

// Get function by index
static inline dynamic_func_t get_function_by_index(int index) {
    if (index >= 0 && index < g_func_count) {
        return g_func_table[index].func;
    }
    return NULL;
}

// ===== NATIVE METHOD RESOLUTION =====

// Resolver for objective-c methods
typedef struct {
    Class cls;
    SEL sel;
    IMP imp;
    BOOL isInstance;
} method_resolver_t;

// Add method dynamically
static inline BOOL add_method_dynamic(Class cls, SEL sel, IMP imp, const char *typeEncoding) {
    if (!cls || !sel || !imp) return NO;
    
    class_addMethod(cls, sel, imp, typeEncoding);
    return YES;
}

// Replace method
static inline BOOL replace_method_dynamic(Class cls, SEL sel, IMP imp, const char *typeEncoding) {
    if (!cls || !sel || !imp) return NO;
    
    Method method = class_getInstanceMethod(cls, sel);
    if (method) {
        method_setImplementation(method, imp);
    } else {
        class_addMethod(cls, sel, imp, typeEncoding);
    }
    return YES;
}

// ===== INTERFACE CHECKS =====

// Check if class exists
static inline BOOL class_exists(const char *className) {
    return objc_getClass(className) != nil;
}

// Check if method exists
static inline BOOL method_exists(Class cls, SEL sel) {
    if (!cls || !sel) return NO;
    return class_respondsToSelector(cls, sel);
}

// Get all classes
static inline NSArray *get_all_classes(void) {
    NSMutableArray *classes = [NSMutableArray new];
    unsigned int count = 0;
    Class *classList = objc_copyClassList(&count);
    
    for (unsigned int i = 0; i < count; i++) {
        [classes addObject:NSStringFromClass(classList[i])];
    }
    
    free(classList);
    return classes;
}

// ===== PROCESS INFO =====

// Get process ID
static inline pid_t get_process_id(void) {
    return getpid();
}

// Get parent process ID
static inline pid_t get_parent_process_id(void) {
    return getppid();
}

// Get process name
static inline NSString* get_process_name(void) {
    return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"] ?: @"Unknown";
}

// ===== MEMORY REGION INFO =====

// Check if address is valid
static inline BOOL is_valid_address(const void *addr) {
    if (!addr) return NO;
    
    vm_address_t address = (vm_address_t)addr;
    vm_size_t size = 0;
    vm_region_basic_info_64_t info;
    mach_port_t port = mach_task_self();
    mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
    
    kern_return_t result = vm_region_64(port, &address, &size, VM_REGION_BASIC_INFO_64, 
                                       (vm_region_info_t)&info, &count, NULL);
    
    return result == KERN_SUCCESS;
}

// Get memory region info
static inline BOOL get_memory_region(const void *addr, vm_address_t *start, vm_size_t *size) {
    if (!addr) return NO;
    
    vm_address_t address = (vm_address_t)addr;
    vm_size_t regionSize = 0;
    vm_region_basic_info_64_t info;
    mach_port_t port = mach_task_self();
    mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
    
    kern_return_t result = vm_region_64(port, &address, &regionSize, VM_REGION_BASIC_INFO_64, 
                                       (vm_region_info_t)&info, &count, NULL);
    
    if (result == KERN_SUCCESS) {
        if (start) *start = address;
        if (size) *size = regionSize;
        return YES;
    }
    
    return NO;
}

// ===== THREAD MANAGEMENT =====

// Get current thread
static inline thread_t get_current_thread(void) {
    return mach_thread_self();
}

// Suspend thread
static inline kern_return_t suspend_thread(thread_t thread) {
    return thread_suspend(thread);
}

// Resume thread
static inline kern_return_t resume_thread(thread_t thread) {
    return thread_resume(thread);
}

// ===== SIGNAL HANDLING =====

// Signal handler type
typedef void (*signal_handler_t)(int);

// Set signal handler
static inline void set_signal_handler(int signum, signal_handler_t handler) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handler;
    sigaction(signum, &action, NULL);
}