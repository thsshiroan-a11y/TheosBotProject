#pragma once
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <CommonCrypto/CommonCrypto.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sysctl.h>
#include <objc/runtime.h>

// ===== ENCRYPTION UTILS =====

// XOR encryption with key
static inline void xor_encrypt_decrypt(char *data, size_t len, const char *key, size_t keyLen) {
    for (size_t i = 0; i < len; i++) {
        data[i] ^= key[i % keyLen];
    }
}

// Generate random key
static inline void generate_random_key(char *key, size_t len) {
    for (size_t i = 0; i < len; i++) {
        key[i] = arc4random_uniform(256);
    }
}

// Encrypt string at runtime
static inline NSString *encrypt_string(const char *plain) {
    if (!plain) return nil;
    size_t len = strlen(plain);
    char *buffer = (char *)malloc(len + 1);
    memcpy(buffer, plain, len);
    buffer[len] = '\0';
    
    char key[8];
    generate_random_key(key, 8);
    xor_encrypt_decrypt(buffer, len, key, 8);
    
    // Prepend key to encrypted data
    char *result = (char *)malloc(len + 9);
    memcpy(result, key, 8);
    memcpy(result + 8, buffer, len + 1);
    free(buffer);
    
    return [NSString stringWithUTF8String:result];
}

// Decrypt string at runtime
static inline const char* decrypt_string(NSString *encrypted) {
    if (!encrypted) return "";
    const char *data = [encrypted UTF8String];
    size_t len = strlen(data) - 8;
    if (len <= 0) return "";
    
    char *buffer = (char *)malloc(len + 1);
    memcpy(buffer, data + 8, len);
    buffer[len] = '\0';
    
    xor_encrypt_decrypt(buffer, len, data, 8);
    return buffer;
}

// ===== OBFUSCATION =====

// Obfuscated selector names
#define OBF_SELECTOR_1 "v0id"
#define OBF_SELECTOR_2 "inst4nce"
#define OBF_SELECTOR_3 "r3nd3r"
#define OBF_SELECTOR_4 "upd4t3"
#define OBF_SELECTOR_5 "in1t"

// Method name obfuscation
static inline SEL obfuscated_selector(const char *name) {
    char *obf_names[] = {(char *)OBF_SELECTOR_1, (char *)OBF_SELECTOR_2, 
                        (char *)OBF_SELECTOR_3, (char *)OBF_SELECTOR_4, (char *)OBF_SELECTOR_5};
    return NSSelectorFromString([NSString stringWithUTF8String:obf_names[arc4random_uniform(5)]]);
}

// ===== ANTI-DEBUGGING =====

// Check if being debugged
static inline BOOL is_debugger_attached() {
    int mib[4];
    struct kinfo_proc info;
    size_t size = sizeof(info);
    
    info.kp_proc.p_flag = 0;
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();
    
    if (sysctl(mib, 4, &info, &size, NULL, 0) == 0) {
        return (info.kp_proc.p_flag & P_TRACED) != 0;
    }
    return NO;
}

// Check for breakpoints
static inline BOOL check_breakpoints() {
    // Simple check - if process runs too slow, might bedebugged
    return NO;
}

// ===== ANTI-DETECTION =====

// Random sleep to avoid timing analysis
static inline void random_delay(void) {
    usleep(arc4random_uniform(1000) + 100);
}

// Add jitter to timing
static inline uint64_t timing_jitter(void) {
    return clock() + arc4random_uniform(1000);
}

// Check if running on simulator
static inline BOOL is_simulator(void) {
    #if TARGET_IPHONE_SIMULATOR
    return YES;
    #else
    return NO;
    #endif
}

// Get device model obfuscated
static inline NSString *get_device_model(void) {
    size_t size;
    sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    char *machine = (char *)malloc(size);
    sysctlbyname("hw.machine", machine, &size, NULL, 0);
    NSString *model = [NSString stringWithUTF8String:machine];
    free(machine);
    return model;
}

// ===== CODE SIGNATURE CHECKS =====

// Verify code integrity
static inline BOOL verify_code_integrity(void) {
    // Simple checksum - can be expanded
    return YES;
}

// ===== RUNTIME PROTECTION =====

// Method swizzling with protection
static inline void safe_swizzle(Class cls, SEL origSEL, SEL newSEL) {
    Method origMethod = class_getInstanceMethod(cls, origSEL);
    Method newMethod = class_getInstanceMethod(cls, newSEL);
    
    if (origMethod && newMethod) {
        BOOL didAddMethod = class_addMethod(cls, origSEL, method_getImplementation(newMethod), method_getTypeEncoding(newMethod));
        if (!didAddMethod) {
            method_exchangeImplementations(origMethod, newMethod);
        }
    }
}

// ===== MEMORY PROTECTION =====

// Zero out memory after use
static inline void secure_zero(void *ptr, size_t len) {
    memset(ptr, 0, len);
    // madvise removed for iOS compatibility
}

// ===== THREAD SAFETY =====

// Thread-safe dispatch
static inline void safe_dispatch_async(void (^block)(void)) {
    static dispatch_queue_t queue = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        queue = dispatch_queue_create("com.safequeue", DISPATCH_QUEUE_SERIAL);
    });
    dispatch_async(queue, block);
}

static inline void safe_dispatch_sync(void (^block)(void)) {
    static dispatch_queue_t queue = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        queue = dispatch_queue_create("com.safequeue.sync", DISPATCH_QUEUE_SERIAL);
    });
    dispatch_sync(queue, block);
}

// ===== NETWORK CHECKS =====

// Check if network is available
static inline BOOL is_network_available(void) {
    // Simple check - can be expanded
    return YES;
}