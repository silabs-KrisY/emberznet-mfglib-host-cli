/***************************************************************************//**
 * @file app.c
 * @brief Callbacks implementation and application specific code.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/

#include "af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/util.h"
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "stack/include/trust-center.h"
#include "zap-cluster-command-parser.h"
#include <stdlib.h>

#include "sl_cli.h" //custom CLI
#include "sl_cli_handles.h"

#define MAX_CTUNE 0x1FF


// The number of tokens that can be written using ezspSetToken and read using
// ezspGetToken.
#define MFGSAMP_NUM_EZSP_TOKENS 8
// The size of the tokens that can be written using ezspSetToken and read using
// ezspGetToken.
#define MFGSAMP_EZSP_TOKEN_SIZE 8
// The number of manufacturing tokens.
#define MFGSAMP_NUM_EZSP_MFG_TOKENS 11
// The size of the largest EZSP Mfg token, EZSP_MFG_CBKE_DATA please refer to
// app/util/ezsp/ezsp-enum.h.
#define MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE 92

extern EmberStatus emberAfTrustCenterStartNetworkKeyUpdate(void);

//---------------------
// CLI
void mfgSetCtuneToken(sl_cli_command_arg_t *arguments);
void mfgGetCtuneToken(sl_cli_command_arg_t *arguments);
void mfgSetCtuneValue(sl_cli_command_arg_t *arguments);
void mfgGetCtuneValue(sl_cli_command_arg_t *arguments);
void getInfoCommand(sl_cli_command_arg_t *arguments);
void getCCAThreshold(sl_cli_command_arg_t *arguments);

static const sl_cli_command_info_t cmd_setCtuneToken = \
    SL_CLI_COMMAND(mfgSetCtuneToken,
                   "set Ctune Token (non-volatile) to <ctune>",
                   "",
                   {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cmd_getCtuneToken = \
   SL_CLI_COMMAND(mfgGetCtuneToken,
                  "get Ctune Token (non-volatile) value",
                  "",
                  {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cmd_setCtuneValue = \
    SL_CLI_COMMAND(mfgSetCtuneValue,
                   "set volatile Ctune value (volatile) to <ctune>",
                   "",
                   {SL_CLI_ARG_UINT16, SL_CLI_ARG_END, });

static const sl_cli_command_info_t cmd_getCtuneValue = \
   SL_CLI_COMMAND(mfgGetCtuneValue,
                  "get Ctune Token value",
                  "",
                  {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cmd_getinfo = \
   SL_CLI_COMMAND(getInfoCommand,
                  "Display the XNCP information on the CLI",
                  "",
                  {SL_CLI_ARG_END, });

static const sl_cli_command_info_t cmd_getCCaThreshold = \
   SL_CLI_COMMAND(getCCAThreshold,
                  "Get the CCA value being used by the stack",
                  "",
                  {SL_CLI_ARG_END, });

// Create the array of commands, containing three elements in this example
static sl_cli_command_entry_t a_table[] = {
  { "setCtuneToken", &cmd_setCtuneToken, false },
  { "getCtuneToken", &cmd_getCtuneToken, false },
  { "setCtuneValue", &cmd_setCtuneValue, false },
  { "getCtuneValue", &cmd_getCtuneValue, false },
  { "get-info", &cmd_getinfo, false },
  { "getCcaThreshold", &cmd_getCCaThreshold, false },
  { NULL, NULL, false },
};

// Create the command group at the top level
static sl_cli_command_group_t a_group_0 = {
  { NULL },
  false,
  a_table
};

void mfgSetCtuneToken(sl_cli_command_arg_t *arguments)
{
	uint16_t ctune = (uint16_t)sl_cli_get_argument_uint16(arguments, 0);
	uint8_t tokenData[2];

  if (ctune > MAX_CTUNE)
  {
    sl_zigbee_app_debug_print("Error: crystal tuning value out of range. Input 0x%x is greater than max 0x%x\r\n\r\n",
  	                        ctune, MAX_CTUNE);
    return;
  }
	tokenData[1] = (uint8_t)(ctune >> 8);
	tokenData[0] = (uint8_t)(ctune);
	ezspSetMfgToken(EZSP_MFG_CTUNE, 2, tokenData);
}

void mfgGetCtuneToken(sl_cli_command_arg_t *arguments)
{
	uint8_t tokenData[8];

	ezspGetMfgToken(EZSP_MFG_CTUNE, tokenData);
  sl_zigbee_app_debug_print("mfg get Ctune Token 0x%02x%02x\r\n\r\n", tokenData[1], tokenData[0]);
}

void mfgSetCtuneValue(sl_cli_command_arg_t *arguments)
{
	uint16_t ctune = (uint16_t)sl_cli_get_argument_uint16(arguments, 0);

  if (ctune > MAX_CTUNE)
  {
    sl_zigbee_app_debug_print("Error: crystal tuning value out of range. Input 0x%04x is greater than max 0x%04x\r\n\r\n",
  	                        ctune, MAX_CTUNE);
    return;
  }

	EzspStatus ezspstatus = ezspSetConfigurationValue(EZSP_CONFIG_CTUNE_VALUE,ctune);
  if (ezspstatus != EZSP_SUCCESS) {
    sl_zigbee_app_debug_print("Error 0x%04x setting ctune value\r\n\r\n",
  	                        ezspstatus);
  }
}

void mfgGetCtuneValue(sl_cli_command_arg_t *arguments)
{
	uint16_t ctune = 0xFFFF;

	EzspStatus ezspstatus = ezspGetConfigurationValue(EZSP_CONFIG_CTUNE_VALUE, &ctune);
  if (ezspstatus != EZSP_SUCCESS) {
    sl_zigbee_app_debug_print("Error 0x%04x getting ctune value\r\n\r\n",
                            ezspstatus);
  } else {
    sl_zigbee_app_debug_print("mfg get Ctune Value 0x%04x\r\n\r\n", ctune);
  }
}
//

//----------------------
// ZCL commands handling

static void ias_ace_cluster_arm_command_handler(uint8_t armMode,
                                                uint8_t* armDisarmCode,
                                                uint8_t zoneId)
{
  uint16_t armDisarmCodeLength = emberAfStringLength(armDisarmCode);
  EmberNodeId sender = emberGetSender();
  uint16_t i;

  sl_zigbee_app_debug_print("IAS ACE Arm Received %04X", armMode);

  // Start i at 1 to skip over leading character in the byte array as it is the
  // length byte
  for (i = 1; i < armDisarmCodeLength; i++) {
    sl_zigbee_app_debug_print("%c", armDisarmCode[i]);
  }
  sl_zigbee_app_debug_print(" %02X\n", zoneId);

  emberAfFillCommandIasAceClusterArmResponse(armMode);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, sender);
}

static void ias_ace_cluster_bypass_command_handler(uint8_t numberOfZones,
                                                   uint8_t* zoneIds,
                                                   uint8_t* armDisarmCode)
{
  EmberNodeId sender = emberGetSender();
  uint8_t i;

  sl_zigbee_app_debug_print("IAS ACE Cluster Bypass for zones ");

  for (i = 0; i < numberOfZones; i++) {
    sl_zigbee_app_debug_print("%d ", zoneIds[i]);
  }
  sl_zigbee_app_debug_print("\n");

  emberAfFillCommandIasAceClusterBypassResponse(numberOfZones,
                                                zoneIds,
                                                numberOfZones);
  emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, sender);
}

static uint32_t zcl_ias_ace_cluster_server_command_handler(sl_service_opcode_t opcode,
                                                           sl_service_function_context_t *context)
{
  (void)opcode;

  EmberAfClusterCommand *cmd = (EmberAfClusterCommand *)context->data;

  switch (cmd->commandId) {
    case ZCL_ARM_COMMAND_ID:
    {
      sl_zcl_ias_ace_cluster_arm_command_t cmd_data;

      if (zcl_decode_ias_ace_cluster_arm_command(cmd, &cmd_data)
          != EMBER_ZCL_STATUS_SUCCESS) {
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
      }

      ias_ace_cluster_arm_command_handler(cmd_data.armMode,
                                          cmd_data.armDisarmCode,
                                          cmd_data.zoneId);
      return EMBER_ZCL_STATUS_SUCCESS;
    }
    case ZCL_BYPASS_COMMAND_ID:
    {
      sl_zcl_ias_ace_cluster_bypass_command_t cmd_data;

      if (zcl_decode_ias_ace_cluster_bypass_command(cmd, &cmd_data)
          != EMBER_ZCL_STATUS_SUCCESS) {
        return EMBER_ZCL_STATUS_UNSUP_COMMAND;
      }

      ias_ace_cluster_bypass_command_handler(cmd_data.numberOfZones,
                                             cmd_data.zoneIds,
                                             cmd_data.armDisarmCode);

      return EMBER_ZCL_STATUS_SUCCESS;
    }
  }

  return EMBER_ZCL_STATUS_UNSUP_COMMAND;
}

//----------------------
// Implemented Callbacks

/** @brief Init
 * Application init function
 */
void emberAfMainInitCallback(void)
{
  // Subscribe to ZCL commands for the IAS_ACE cluster, server side.
  sl_zigbee_subscribe_to_zcl_commands(ZCL_IAS_ACE_CLUSTER_ID,
                                      0xFFFF,
                                      ZCL_DIRECTION_CLIENT_TO_SERVER,
                                      zcl_ias_ace_cluster_server_command_handler);

  //  install the custom commands.
  sl_cli_command_add_command_group(sl_cli_example_handle, &a_group_0);

}

//-------------------------------------
// Custom CLI commands and related code


// The manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h.
// The names are enumerated here to make it easier for the user.
static const char * ezspMfgTokenNames[] =
{
  "EZSP_MFG_CUSTOM_VERSION...",
  "EZSP_MFG_STRING...........",
  "EZSP_MFG_BOARD_NAME.......",
  "EZSP_MFG_MANUF_ID.........",
  "EZSP_MFG_PHY_CONFIG.......",
  "EZSP_MFG_BOOTLOAD_AES_KEY.",
  "EZSP_MFG_ASH_CONFIG.......",
  "EZSP_MFG_EZSP_STORAGE.....",
  "EZSP_STACK_CAL_DATA.......",
  "EZSP_MFG_CBKE_DATA........",
  "EZSP_MFG_INSTALLATION_CODE"
};

// Called to dump all of the tokens. This dumps the indices, the names,
// and the values using ezspGetToken and ezspGetMfgToken. The indices
// are used for read and write functions below.
void mfgappTokenDump(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  EmberStatus status;
  uint8_t tokenData[MFGSAMP_EZSP_TOKEN_MFG_MAXSIZE];
  uint8_t index, i, tokenLength;

  // first go through the tokens accessed using ezspGetToken
  sl_zigbee_app_debug_print("(data shown little endian)\n");
  sl_zigbee_app_debug_print("Tokens:\n");
  sl_zigbee_app_debug_print("idx  value:\n");
  for (index = 0; index < MFGSAMP_NUM_EZSP_TOKENS; index++) {
    // get the token data here
    status = ezspGetToken(index, tokenData);
    sl_zigbee_app_debug_print("[%d]", index);
    if (status == EMBER_SUCCESS) {
      // Print out the token data
      for (i = 0; i < MFGSAMP_EZSP_TOKEN_SIZE; i++) {
        sl_zigbee_app_debug_print(" %02X", tokenData[i]);
      }
      sl_zigbee_app_debug_print("\n");
    } else {
      // handle when ezspGetToken returns an error
      sl_zigbee_app_debug_print(" ... error 0x%02X ...\n", status);
    }
  }

  // now go through the tokens accessed using ezspGetMfgToken
  // the manufacturing tokens are enumerated in app/util/ezsp/ezsp-protocol.h
  // this file contains an array (ezspMfgTokenNames) representing the names.
  sl_zigbee_app_debug_print("Manufacturing Tokens:\n");
  sl_zigbee_app_debug_print("idx  token name                 len   value\n");
  for (index = 0; index < MFGSAMP_NUM_EZSP_MFG_TOKENS; index++) {
    // ezspGetMfgToken returns a length, be careful to only access
    // valid token indices.
    tokenLength = ezspGetMfgToken(index, tokenData);
    sl_zigbee_app_debug_print("[%x] %s: 0x%x:\n",
                              index, ezspMfgTokenNames[index], tokenLength);

    // Print out the token data
    for (i = 0; i < tokenLength; i++) {
      if ((i != 0) && ((i % 8) == 0)) {
        sl_zigbee_app_debug_print("\n                                    :");
      }
      sl_zigbee_app_debug_print(" %02X", tokenData[i]);
    }
    sl_zigbee_app_debug_print("\n");
  }
  sl_zigbee_app_debug_print("\n");
}

void changeNwkKeyCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  EmberStatus status = emberAfTrustCenterStartNetworkKeyUpdate();

  if (status != EMBER_SUCCESS) {
    sl_zigbee_app_debug_print("Change Key Error %x\n", status);
  } else {
    sl_zigbee_app_debug_print("Change Key Success\n");
  }
}

static void dcPrintKey(uint8_t label, uint8_t *key)
{
  uint8_t i;
  sl_zigbee_app_debug_print("key %x: \n", label);
  for (i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
    sl_zigbee_app_debug_print("%02X", key[i]);
  }
  sl_zigbee_app_debug_print("\n");
}

void printNextKeyCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  EmberKeyStruct nextNwkKey;
  EmberStatus status;

  status = emberGetKey(EMBER_NEXT_NETWORK_KEY,
                       &nextNwkKey);

  if (status != EMBER_SUCCESS) {
    sl_zigbee_app_debug_print("Error getting key\n");
  } else {
    dcPrintKey(1, nextNwkKey.key.contents);
  }
}

void versionCommand(sl_cli_command_arg_t *arguments)
{
  (void)arguments;

  sl_zigbee_app_debug_print("Version:  0.1 Alpha\n");
  sl_zigbee_app_debug_print(" %s\n", __DATE__);
  sl_zigbee_app_debug_print(" %s\n", __TIME__);
  sl_zigbee_app_debug_print("\n");
#ifdef EMBER_TEST
  sl_zigbee_app_debug_print("Print formatter test : 0x%x=0x12, 0x%02x=0x1234 0x%04x=0x12345678\n",
                            0x12, 0x1234, 0x12345678);
#endif
}

void setTxPowerCommand(sl_cli_command_arg_t *arguments)
{
  int8_t dBm = sl_cli_get_argument_int8(arguments, 0);

  emberSetRadioPower(dBm);
}

void getInfoCommand(sl_cli_command_arg_t *arguments)
{
  uint16_t version, manufacturerId;
  EmberStatus status;

  status = ezspGetXncpInfo(&manufacturerId, &version);



  sl_zigbee_app_debug_print("Get XNCP info: status: 0x%X", status);
  sl_zigbee_app_debug_print("  manufacturerId: 0x%X, version: 0x%X\n",
                     manufacturerId, version);
}

void getCCAThreshold(sl_cli_command_arg_t *arguments)
{
  uint8_t cca_threshold[2];
  uint8_t cca_threshold_len=2;

  // Get the CCA threshold used by the stack (either default value or from the TOKEN_MFG_CCA_THRESHOLD)
  // value is 2s complement representation of the CCA threshold in dBm. The default is 0xb5 => - 75 dBm

  EzspStatus ezspstatus = ezspGetValue(EZSP_VALUE_CCA_THRESHOLD, &cca_threshold_len, cca_threshold);
  if (ezspstatus == EZSP_SUCCESS) {
    sl_zigbee_app_debug_print("cca threshold value 0x%x\r\n\r\n", cca_threshold[0]);
  } else {
    sl_zigbee_app_debug_print("Error 0x%04x getting cca threshold value\r\n\r\n",
                              ezspstatus);
  }
}
