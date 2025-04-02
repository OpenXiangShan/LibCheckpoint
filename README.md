# LibCheckpoint

LibCheckpoint is a restorer for "rvgcpt" checkpoints, restoring the in-memory architectural state into registers.

It provides the following functions:
- Restore a checkpoint that uses Protobuf to define the memory layout (max cores: 128).
- Restore a dual-core "rv64gcbvh" checkpoint, generated using QEMU (https://github.com/OpenXiangShan/qemu/tree/9.0.0_checkpoint).
- Restore a single-core "rv64gcbvh" checkpoint, generated using either QEMU (https://github.com/OpenXiangShan/qemu/tree/9.0.0_checkpoint) or NEMU (https://github.com/OpenXiangShan/NEMU)
- Add limited support for restoring from M-mode (it is best not to generate checkpoints on bare-metal programs, as this makes debugging and support difficult).
- Output the in-memory architectural state (before recovery).
- Block core N (used for debugging).

## How to Use

1. Initialize the environment
```
pip install --upgrade protobuf grpcio-tools
git submodule update --init
```

2. Build for restoring a checkpoint using Protobuf (stable)
```
./configure && make -j
```

3. Build for a dual core QEMU checkpoint with serialized data at 0x80300000~0x80800000 (stable)
```
./configure --mode=dual_core && make -j

```

4. Build the restorer than can pause a core's operation (stable)
```bash
./configure --stop_cpu=N && make -j
```

5. Build restorer that can display serialize data from memory before unserialize to hardware (stable)
```bash
./configure --display_cpu=0 # n could be any hartid, one appropriate approach is to suspend one of the cores in a dual-core checkpoint and output the serialized data from the other core
```

6. Build for linking to the next-level bootloader (stable)
```
./configure --gcpt_payload=/path/to/bbl.bin # default: put next level bootloader at address 0x80100000
./configure --gcpt_payload=/path/to/bbl.bin --gcpt_payload_position=0x???????? # point to link address
```

7. Debugging ( In Process )

still in process...

- The above options are not mutually exclusive and can be used in any combination

## Reference
- protobuf (https://github.com/nanopb/nanopb)
- printf (https://github.com/mpaland/printf)
- gcpt_restore (https://github.com/OpenXiangShan/LibCheckpointAlpha)

## FAQ:
- warning: build/gcpt has a LOAD segment with RWX permissions
    - this warning does not affect any normal use
