# ipwm

ipwm is a lightweight tool to adjust PWM frequency on i915.

Read [the article](https://0x0.st/Dy) to understand how it works.

## Building and Installing

Run `make && make install` to build and install ipwm to your system.

## Configuration

The configuration file is placed to `/etc/ipwm.conf` by default.
You can edit the following parameters:

- `register` — `0xc8254` is default value, you don't need to change it in most cases.
- `period` — adjusts PWM.

After editing the parameters you can run `ipwm apply` from root.
You can view the current value running `ipwm read` from root.
The value will also be automatically applied after i915 module load.

## Troubleshooting

### Value Doesn't Change After Boot

i915 module was probably loaded before mounting filesystems.
You can disable early load of i915 module or copy the tool to initramfs.

Arch Linux users can add `ipwm` hook to `mkinitcpio.conf(5)`.
