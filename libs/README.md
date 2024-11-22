### Folders structure:
```
.
├── libs.mk         - makefile to include in project
├── make            - default makefile rules and utilities
├── sys             - system level functions and utils - logging, descriptors, etc
├── targets         - common targets bsp functions
│   ├── bsp
│   ├── nucleo_f429zi
│   └── nucleo_h743zi
├── thirdparty      - third-party libraries
│   ├── FreeRTOS
│   ├── LwIP
│   ├── lan8742
│   ├── mlib
│   ├── nanopb
│   └── printf
├── utils           - debugging and flashing utils
│   ├── FreeRTOS
│   ├── PyCortexMDebug
│   ├── elf_size_analyze.py
│   ├── fw_bundler  - firmware bundler (submodule)
│   └── protobuf_test
└── vendor          - hal and cmsis libraries
    ├── CMSIS
    ├── STM32F4
    └── STM32H7
```