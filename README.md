# LibCheckpoint

LibCheckpoint is a restorer of "rvgcpt checkpoint" for restoring in-memory architectural state into registers.

Provides the following functions:
- restore checkpoint which using protobuf point to memory layout (max cores: 128)
- restore dual core "rvgcpt checkpoint" which using qemu generate (https://github.com/OpenXiangShan/qemu/tree/checkpoint) (Currently the most commonly used)
- limited M-mode checkpoint restore (it is best not to try to generate checkpoints on bare metal programs)
- output the in-memory architecture state (before recovery)
- block core N (using for debug)

## How to

1. Init
```
pip install --upgrade protobuf grpcio-tools
git submodule update --init
```

2. Build for restore checkpoint that using protobuf and 'checkpoint' device (Not officially put into use, only passed testing)
```
make clean && make -j
```

3. Build for qemu dual core checkpoint that put serialize data at 0x80300000~0x80800000 (stable)
```
make clean && make USING_QEMU_DUAL_CORE_SYSTEM=1

```

4. Build restorer that can pause a core's operation (stable)
```bash
make STOP_CPU_N=n # n could be any hartid
```

5. Build restorer that can display serialize data from memory before unserialize to hardware (stable)
```bash
make DISPLAY_CPU_N=0 # n could be any hartid, one appropriate approach is to suspend one of the cores in a dual-core checkpoint and output the serialized data from the other core
```

6. Build for link next level bootloader (stable)
```
make GCPT_PAYLOAD_PATH=/path/to/bbl.bin # default: put next level bootloader at address 0x80100000
make GCPT_PAYLOAD_PATH=/path/to/bbl.bin GCPT_PAYLOAD_POSITION=0x???????? # point to link address
```

- The above options are not mutually exclusive and can be used in any combination

## Reference
- protobuf (https://github.com/nanopb/nanopb)
- printf (https://github.com/mpaland/printf)
- gcpt_restore (https://github.com/OpenXiangShan/LibCheckpointAlpha)

## FAQ:
- warning: build/gcpt has a LOAD segment with RWX permissions
    - this warning does not affect any normal use
