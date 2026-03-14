#ifndef CONFIG_H
#define CONFIG_H

// Build profile defaults (overridden by PlatformIO env build_flags)
#ifndef DCMT_HW_GEN
#define DCMT_HW_GEN 1
#endif

#ifndef DCMT_FEATURE_CLOSED_LOOP
#define DCMT_FEATURE_CLOSED_LOOP 0
#endif

// Bus address is independent of hardware generation.
// Override with a build flag: -DI2C_ADR=<addr>
#ifndef I2C_ADR
#define I2C_ADR 21
#endif

#define VERSION "1.0.0" // firmware version, update when making changes

// Uncomment to enable additional debugging messages:
#define SLICE_DEBUG 1

#ifdef SLICE_DEBUG

#define SLICE_DEBUG_PRINT(...) \
    Serial.print(__VA_ARGS__)

#define SLICE_DEBUG_PRINTLN(...) \
    Serial.println(__VA_ARGS__)

#else
#define SLICE_DEBUG_PRINT(...)
#define SLICE_DEBUG_PRINTLN(...)
#endif

#endif // CONFIG_H
