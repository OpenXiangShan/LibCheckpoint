# How to
- init
```bash
git submodule update --init
```

- Build for restore checkpoint that using protobuf and 'checkpoint' device
```bash
make
```

- Build for qemu checkpoint that put serialize data at 0x80300000~0x80800000
```bash
make USING_QEMU_DUAL_CORE_SYSTEM=1
```

- Build restorer that can pause a core's operation
```bash
make STOP_CPU_N=n # n could be any hartid
```

- Build restorer that can display serialize data from memory before unserialize to hardware

```bash
make DISPLAY_CPU_N=0 # n could be any hartid, one appropriate approach is to suspend one of the cores in a dual-core checkpoint and output the serialized data from the other core
```
