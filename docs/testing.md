---
title: "Testing Instructions"
description: "Hardware and firmware testing checklist and procedures."
---

Testing procedures for this Slice. Covers manual validation and optional automated checks.

## ✅ Hardware Checklist

- [ ] Visual inspection (solder joints, polarity, orientation)
- [ ] No shorts across power rails
- [ ] All headers and connectors populated

## ⚡ Power-On Verification

1. Apply logic power (e.g., 5V).
2. Confirm LED heartbeat (if present).
3. Measure voltage rails at test points.

## 📟 Functional Tests

| Test        | Method                            | Expected Result                     |
| ----------- | --------------------------------- | ----------------------------------- |
| UART output | Connect USB-Serial, open terminal | Data output at expected baud rate   |
| Input test  | Drive digital/PWM input           | MCU registers or reacts accordingly |
| Output test | Scope or multimeter at output pin | Output toggles or changes as coded  |

## 🔁 Firmware Regression (if present)

```bash
pio run -t test
```

## 📷 Test Setup

<!-- insert annotated image of bench setup under docs/assets/ and insert here -->
