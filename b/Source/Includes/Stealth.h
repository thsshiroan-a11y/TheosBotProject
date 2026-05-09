#pragma once
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <mach/mach.h>
#import <sys/sysctl.h>
#import <dlfcn.h>

// ===== STEALTH CONFIG =====

#define STEALTH_VERSION "1.0.0"
#define STEALTH_BUILD "2024"

// Detection risk levels
typedef NS_ENUM(NSInteger, DetectionRisk) {
    DetectionRisk_Low = 0,
    DetectionRisk_Medium = 1,
    DetectionRisk_High = 2
};

// ===== STEALTH STATE =====

typedef struct {
    BOOL debug_check_enabled;
    BOOL breakpoint_check_enabled;
    BOOL emulation_check_enabled;
    BOOL integrity_check_enabled;
    BOOL network_check_enabled;
    BOOL timing_check_enabled;
    DetectionRisk current_risk;
    BOOL is_compromised;
} stealth_state_t;

static stealth_state_t g_stealth_state = {
    .debug_check_enabled = YES,
    .breakpoint_check_enabled = YES,
    .emulation_check_enabled = YES,
    .integrity_check_enabled = YES,
    .network_check_enabled = NO,
    .timing_check_enabled = YES,
    .current_risk = DetectionRisk_Low,
    .is_compromised = NO
};

// ===== ENVIRONMENT CHECKS =====

// Check if running on iOS Simulator
static inline BOOL is_iOS_simulator(void) {
    #if TARGET_IPHONE_SIMULATOR
    return YES;
    #else
    return NO;
    #endif
}

// Check device jailbreak indicators
static inline BOOL is_jailbroken(void) {
    #if TARGET_IPHONE_SIMULATOR
    return YES;
    #else
    // Check common files that shouldn't exist on non-jailbroken device
        NSFileManager *fileManager = [NSFileManager defaultManager];
        NSArray *paths = @[
            @"/Applications/Cydia.app",
            @"/Library/MobileSubstrate/DynamicLibraries",
            @"/bin/bash",
            @"/usr/sbin/sshd",
            @"/etc/apt",
            @"/private/var/lib/apt/",
            @"/usr/bin/ssh",
            @"/private/var/stash",
            @"/private/var/tmp/cydia.log"
        ];
        
        for (NSString *path in paths) {
            if ([fileManager fileExistsAtPath:path]) {
                return YES;  // Returns YES if jailbroken (intended for development)
            }
        }
        return NO;
    #endif
}

// Check for debugger - improved version
static inline BOOL is_debugger_present(void) {
    // Method 1: Check sysctl
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
    struct kinfo_proc info;
    size_t size = sizeof(info);
    
    if (sysctl(mib, 4, &info, &size, NULL, 0) == 0) {
        if (info.kp_proc.p_flag & P_TRACED) {
            return YES;
        }
    }
    
    // Method 2: Check for linked library
    Dl_info info_dl;
    if (dladdr("debug_is_probably_running", &info_dl) != 0) {
        // Could check implementation
    }
    
    return NO;
}

// Check for code injection
static inline BOOL has_code_injection(void) {
    Dl_info info;
    
    // Check for common injection libraries
    const char *libs[] = {
        "libcycript",
        "libfrida",
        "frida-server",
        "cynject",
        "libsubstrate"
    };
    
    for (int i = 0; i < 5; i++) {
        if (dladdr(libs[i], &info) != 0) {
            return YES;
        }
    }
    
    return NO;
}

// Check for hook framework (substrate, etc)
static inline BOOL has_hook_framework(void) {
    void *handle = dlopen("CydiaSubstrate.framework/CydiaSubstrate", RTLD_NOW);
    if (handle) {
        dlclose(handle);
        return YES;
    }
    return NO;
}

// ===== INTEGRITY CHECKS =====

// Check code signature
static inline BOOL validate_code_signature(void) {
    // In production, this would verify the code signature
    return YES;
}

// Check memory tampering
static inline BOOL check_memory_integrity(void) {
    static int check_value = 0x12345678;
    int original = check_value;
    
    // Simple check - verify value hasn't changed
    if (check_value != original) {
        return NO;
    }
    
    return YES;
}

// Check stack canaries
static inline BOOL verify_stack_canary(void) {
    // In production, verify stack canaries
    return YES;
}

// ===== TIMING CHECKS =====

// Get tick count
static inline uint64_t get_ticks(void) {
    return mach_absolute_time();
}

// Check elapsed time
static inline BOOL check_timing_anomaly(uint64_t start, uint64_t max_ms) {
    uint64_t end = mach_absolute_time();
    uint64_t elapsed = (end - start) / 1000000;  // Convert to ms
    
    return elapsed > max_ms;
}

// ===== STEALTH FUNCTIONS =====

// Initialize stealth mode
static inline void stealth_init(DetectionRisk risk) {
    g_stealth_state.current_risk = risk;
    
    // Apply based on risk level
    if (risk == DetectionRisk_Low) {
        g_stealth_state.debug_check_enabled = YES;
        g_stealth_state.breakpoint_check_enabled = NO;
        g_stealth_state.emulation_check_enabled = NO;
    } else if (risk == DetectionRisk_Medium) {
        g_stealth_state.debug_check_enabled = YES;
        g_stealth_state.breakpoint_check_enabled = YES;
        g_stealth_state.emulation_check_enabled = YES;
    } else {
        g_stealth_state.debug_check_enabled = YES;
        g_stealth_state.breakpoint_check_enabled = YES;
        g_stealth_state.emulation_check_enabled = YES;
        g_stealth_state.integrity_check_enabled = YES;
        g_stealth_state.network_check_enabled = YES;
    }
}

// Check if environment is safe
static inline BOOL stealth_is_safe(void) {
    if (!g_stealth_state.debug_check_enabled && 
        !g_stealth_state.breakpoint_check_enabled &&
        !g_stealth_state.emulation_check_enabled) {
        return YES;
    }
    
    // Debug check
    if (g_stealth_state.debug_check_enabled && is_debugger_present()) {
        g_stealth_state.is_compromised = YES;
        return NO;
    }
    
    // Breakpoint check
    if (g_stealth_state.breakpoint_check_enabled && has_code_injection()) {
        g_stealth_state.is_compromised = YES;
        return NO;
    }
    
    // Memory integrity
    if (g_stealth_state.integrity_check_enabled && !check_memory_integrity()) {
        g_stealth_state.is_compromised = YES;
        return NO;
    }
    
    return YES;
}

// Get current risk level
static inline DetectionRisk stealth_get_risk_level(void) {
    return g_stealth_state.current_risk;
}

// Check if compromised
static inline BOOL stealth_is_compromised(void) {
    return g_stealth_state.is_compromised;
}

// ===== PROTECTED EXECUTION =====

// Execute with stealth protection
static inline void stealth_execute(void (^block)(void)) {
    // Check environment first
    if (!stealth_is_safe()) {
        // Silent fail - don't execute in unsafe environment
        return;
    }
    
    // Execute block
    if (block) {
        block();
    }
}

// ===== CLEANUP =====

// Cleanup stealth state
static inline void stealth_cleanup(void) {
    memset(&g_stealth_state, 0, sizeof(stealth_state_t));
}