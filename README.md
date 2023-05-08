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

**NOTE: The CTUNE token can only be set once in software. Once it is set, Simplicity Commander and a debug connection is required to erase and/or change it. So “getCtuneValue” must be used to test the value prior to using “setCtuneToken” to set it permanently.**

## Examples

# CtuneToken and CtuneValue

With the CtuneToken and CtuneValue commands, we can use the Manufacturing Library (refer to [AN1162](https://www.silabs.com/documents/public/application-notes/an1162-using-manufacturing-library.pdf)) to iterate on setting ctune and outputting a CW tone. We can see the frequency of the CW tone change with each ctune value:
 
  Z3Gateway> plugin mfglib start 1
  Z3Gateway> plugin mfglib set-channel 11
  Z3Gateway> plugin mfglib set-power 10 0
  Z3Gateway> setCtuneValue 0x00
  Z3Gateway> plugin mfglib tone start
  Z3Gateway> plugin mfglib tone stop
  Z3Gateway> setCtuneValue 0x80
  Z3Gateway> plugin mfglib tone start
  Z3Gateway> plugin mfglib tone stop

# XNCP Info

To add the xncp version info to your NCP firmware image:
1. Add the Zigbee->NCP->XNCP component to your NCP firmware project
2. Add code like this to app.c in the NCP firmware project:
    #define XNCP_VERSION_NUMBER  0x1234
    #define XNCP_MANUFACTURER_ID 0xABCD
    /** @brief Get XNCP Information
    *
    * This callback enables users to communicate the version number and
    * manufacturer ID of their NCP application to the framework. This information
    * is needed for the EZSP command frame called getXncpInfo. This callback will
    * be called when that frame is received so that the application can report
    * its version number and manufacturer ID to be sent back to the HOST.
    *
    * @param versionNumber The version number of the NCP application.
    * @param manufacturerId The manufacturer ID of the NCP application.
    */
    void emberAfPluginXncpGetXncpInformation(uint16_t *manufacturerId,
                                            uint16_t *versionNumber)
    {
    *versionNumber = XNCP_VERSION_NUMBER;
    *manufacturerId = XNCP_MANUFACTURER_ID;
    }

Here’s what this looks like on Z3Gateway when executing the “get-info” command:
 
    Z3GatewayHost> custom get-info
    Get XNCP info: status: 0x00
    manufacturerId: 0xABCD, version: 0x1234

## Quality - EXPERIMENTAL

This code has not been formally tested and is provided as-is. It is not suitable for production environments. In addition, this code will not be maintained and there may be no bug maintenance planned for these resources. Silicon Labs may update projects from time to time.

## License
 The licensor of this software is Silicon Laboratories Inc. Your use of this software is governed by the terms of Silicon Labs Master Software License Agreement (MSLA) available at www.silabs.com/about-us/legal/master-software-license-agreement. This software is distributed to you in Source Code format and is governed by the sections of the MSLA applicable to Source Code.