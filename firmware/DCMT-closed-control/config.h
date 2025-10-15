#ifndef CONFIG_H
#define CONFIG_H

// define the I2C address for this SLICE device
#define I2C_ADR 14
#define TYPE_ID 2 // SLICE type ID for DCMT!!!

//  Uncomment to enable additional debugging messages:
#define SLICE_DEBUG 1

#define VERSION "1.0.0"

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