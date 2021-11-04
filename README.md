# RadeonSensor - Kext and Gadget to show Radeon GPU temperature on MacOS

![](./imgs/icon.png)

The kext is based on FakeSMCs RadeonMonitor to provide GPU temperature to a dedicated gadget without relying on FakeSMC being installed and can therefore be used with VirtualSMC instead. All credits for the code to read the GPU temperature go to FakeSMC/RadeonMonitor project.

The design of the status bar is based on the AMD PowerGadget to fit together.

## GPU temperature and MacOS
Starting with the Radeon VII, Apple stopped reporting the temperature directly, requiring kexts to step in and implement that feature. For Vega 10 and older, other tools can already display the GPU temperature without the need for additional kexts.

## Components

* RadeonSensor.kext: Required to read the GPU temperature and requires Lilu
* SMCRadeonGPU.kext: Can be used optionally to export GPU temperature to VirtualSMC for monitoring tools to read and requires VirtualSMC
* RadeonGadget.app: Displays GPU temperature in the status bar and requires only the RadeonSensor kext to be loaded

## Supported GPUs

All GPUs starting Radeon HD 7000 series up to the RX 6000 series are supported.


Cedits
* [FakeSMC3](https://github.com/CloverHackyColor/FakeSMC3_with_plugins)
* [SMCAMDProcessor](https://github.com/trulyspinach/SMCAMDProcessor)

![](./imgs/status_bar.png)
