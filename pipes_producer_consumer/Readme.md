# Compiling the OpenCL Kernel

The top-level OpenCL kernels file are device/pipe_writer.cl and device/pipe_reader.cl.

To compile the OpenCL kernel, run:

```
aoc device/pipe_writer.cl device/pipe_reader.cl -o bin/pipes_producer_consumer.aocx --board <board>
```

where ```<board>``` matches the board you want to target. The ```-o bin/hello_world.aocx``` argument is used to place the compiled binary in the location that the host program expects.

If you are unsure of the boards available, use the following command to list available boards: ```aoc --list-boards```


# Compiling for Emulator

To use the emulation flow, the compilation command just needs to be modified slightly:

```
aoc -march=emulator device/pipe_writer.cl device/pipe_reader.cl -o bin/pipes_producer_consumer.aocx  --board <board>
```

where ```--board <board>``` is optional.


# Compiling the Host Program

To compile the host program, run:

```
make
```

The compiled host program will be located at ```bin/host```.


# Running the Host Program

Before running the host program, you should have compiled the OpenCL kernel and the host program. Refer to [this guide](https://www.altera.com/content/dam/altera-www/global/en_US/pdfs/literature/hb/opencl-sdk/aocl_programming_guide.pdf) if you have not completed those steps.

To run the host program on hardware, execute:

```
bin/host
```

# Running with the Emulator

Prior to running the emulation flow, ensure that you have compiled the kernel for emulation. Refer to [this guide](https://www.altera.com/content/dam/altera-www/global/en_US/pdfs/literature/hb/opencl-sdk/aocl_programming_guide.pdf) if you have not done so. Also, please set up your environment for emulation.  

For this example design, the suggested emulation command is:

```
CL_CONTEXT_EMULATOR_DEVICE_ALTERA=1 bin/host
```
