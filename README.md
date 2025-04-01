# LibCheckpoint

LibCheckpoint is a restorer of "rvgcpt checkpoint" for restoring in-memory architectural state into registers.

Provides the following functions:
- restore checkpoint which using protobuf point to memory layout (max cores: 128)
- restore dual core "rv64gcbvh checkpoint" which using QEMU (https://github.com/OpenXiangShan/qemu/tree/9.0.0_checkpoint) generate.
- restore single core "rv64gcbvh checkpoint" which using QEMU (https://github.com/OpenXiangShan/qemu/tree/9.0.0_checkpoint) or NEMU(https://github.com/OpenXiangShan/NEMU) generate.
- limited M-mode checkpoint restore (it is best not to try to generate checkpoints on bare metal programs, it makes debugging and support difficult)
- output the in-memory architecture state (before recovery)
- block core N (using for debug)

## How to

1. Init
```
pip install --upgrade protobuf grpcio-tools
git submodule update --init
```

2. Build for restore checkpoint that using protobuf (stable)
```
./configure && make -j
```

3. Build for qemu dual core checkpoint that put serialize data at 0x80300000~0x80800000 (stable)
```
./configure --mode=dual_core && make -j

```

4. Build restorer that can pause a core's operation (stable)
```bash
./configure --stop_cpu=N && make -j
```

5. Build restorer that can display serialize data from memory before unserialize to hardware (stable)
```bash
./configure --display_cpu=0 # n could be any hartid, one appropriate approach is to suspend one of the cores in a dual-core checkpoint and output the serialized data from the other core
```

6. Build for link next level bootloader (stable)
```
./configure --gcpt_payload=/path/to/bbl.bin # default: put next level bootloader at address 0x80100000
./configure --gcpt_payload=/path/to/bbl.bin --gcpt_payload_position=0x???????? # point to link address
```

7. Debug ( in process )

still in process...

- The above options are not mutually exclusive and can be used in any combination

## Reference
- protobuf (https://github.com/nanopb/nanopb)
- printf (https://github.com/mpaland/printf)
- gcpt_restore (https://github.com/OpenXiangShan/LibCheckpointAlpha)

## FAQ:
- warning: build/gcpt has a LOAD segment with RWX permissions
    - this warning does not affect any normal use
