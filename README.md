# emberznet-mfglib-host-cli

This is an example app.c file for the Z3Gateway EmberZNet posix host application which adds some custom CLI commands that are useful for manufacturing purposes.

## Getting Started

### Prerequisites

1. EmberZNet 7.0 or higher (tested on EmberZNet 7.0.2).
2. Linux/Posix build environment (OSX, Raspberry Pi, etc.) or Windows (Cygwin/MinGW).
3. Mighty Gecko device running an EmberZNet NCP (EZSP) firmware and a bootloader. Make sure the NCP firmware includes the "Manufacturing Library" component when it is built.
4. Z3Gateway project created within Simplicity Studio. Make sure to include the "Manufacturing Library CLI" component.

### Installung

Simply merge or copy this app.c into your Z3Gateway project and rebuilt for the host target.

## Using

This example adds the following custom commands:

* setCtuneToken <uint16>        set Ctune Token (non-volatile) to <ctune>
    Example: setCtuneToken 0x80
* getCtuneToken                 get Ctune Token (non-volatile) value
* setCtuneValue <uint16)>        set volatile Ctune value (volatile) to <ctune>
    Example: setCtuneValue 0x80
* getCtuneValue                 get Ctune Token value
* get-info                      get XNCP info defined by callback on the NCP firmware

## Quality - EXPERIMENTAL

This code has not been formally tested and is provided as-is. It is not suitable for production environments. In addition, this code will not be maintained and there may be no bug maintenance planned for these resources. Silicon Labs may update projects from time to time.

## License
 The licensor of this software is Silicon Laboratories Inc. Your use of this software is governed by the terms of Silicon Labs Master Software License Agreement (MSLA) available at www.silabs.com/about-us/legal/master-software-license-agreement. This software is distributed to you in Source Code format and is governed by the sections of the MSLA applicable to Source Code.