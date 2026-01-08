
##Project Setup & Architecture
```
rtc-digital-clock/
├── Core/
│   ├── Inc/
│   │   ├── app/
│   │   │   ├── clock_manager.h       # Timekeeping logic
│   │   │   ├── alarm_manager.h       # Alarm functionality
│   │   │   └── display_manager.h     # Display interface
│   │   ├── drivers/
│   │   │   ├── rtc.h                 # RTC hardware abstraction
│   │   │   ├── button.h              # (Reuse from previous)
│   │   │   ├── display.h             # LCD/OLED/UART display
│   │   │   └── backup_domain.h       # Backup register handling
│   │   └── system/
│   │       ├── systick.h             # (Reuse)
│   │       ├── interrupts.h          # NVIC setup
│   │       └── power.h               # Low-power modes
│   ├── Src/
│   │   ├── app/                      # Mirror Inc/app/
│   │   ├── drivers/                  # Mirror Inc/drivers/
│   │   └── system/                   # Mirror Inc/system/
│   └── Startup/
├── config/
│   ├── board_config.h                # Hardware pins
│   └── clock_config.h                # Clock tree configuration
├── docs/
│   ├── architecture.md               # System design
│   ├── rtc_setup.md                  # RTC configuration
│   └── display_protocol.md           # Display interface
└── tests/
    └── rtc_test.c                    # Unit tests
```

##System Block Diagram:
```
┌─────────────────────────────────────────────────┐
│              Application Layer                   │
│  Clock Manager  │ Alarm Manager │ Display Mgr   │
├─────────────────────────────────────────────────┤
│            Time Setting State Machine           │
├─────────────────────────────────────────────────┤
│   RTC Sec IRQ  │ RTC Alarm IRQ │ Button IRQs   │
├─────────────────────────────────────────────────┤
│        Backup Domain      │        RTC          │
└─────────────────────────────────────────────────┘
```