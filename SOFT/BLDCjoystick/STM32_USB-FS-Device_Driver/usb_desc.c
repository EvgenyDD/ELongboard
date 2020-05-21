/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : usb_desc.c
* Author             : MCD Application Team
* Version            : V3.0.1
* Date               : 04/27/2009
* Description        : Descriptors for Virtual Com Port Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
/* USB Standard Device Descriptor */
const uint8_t Virtual_Com_Port_DeviceDescriptor[] =
  {
    0x12,   /* bLength */
    USB_DEVICE_DESCRIPTOR_TYPE,     /* bDescriptorType */
    0x00,
    0x02,   /* bcdUSB = 2.00 */
    0x02,   /* bDeviceClass: CDC */
    0x00,   /* bDeviceSubClass */
    0x00,   /* bDeviceProtocol */
    0x40,   /* bMaxPacketSize0 */
    0x83,
    0x04,   /* idVendor = 0x0483 */
    0x40,
    0x57,   /* idProduct = 0x7540 */
    0x00,
    0x02,   /* bcdDevice = 2.00 */
    1,              /* Index of string descriptor describing manufacturer */
    2,              /* Index of string descriptor describing product */
    3,              /* Index of string descriptor describing the device's serial number */
    0x01    /* bNumConfigurations */
  };

const uint8_t Virtual_Com_Port_ConfigDescriptor[] =
  {
    /*Configuation Descriptor*/
    0x09,   /* bLength: Configuation Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
    VIRTUAL_COM_PORT_SIZ_CONFIG_DESC,       /* wTotalLength:no of returned bytes */
    0x00,
    0x02,   /* bNumInterfaces: 2 interface */
    0x01,   /* bConfigurationValue: Configuration value */
    0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,   /* bmAttributes: self powered */
    0x32,   /* MaxPower 0 mA */
    /*Interface Descriptor*/
    0x09,   /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x00,   /* iInterface: */
    /*Header Functional Descriptor*/
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,
    /*Call Managment Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */
    0x01,   /* bDataInterface: 1 */
    /*ACM Functional Descriptor*/
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */
    /*Union Functional Descriptor*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x01,   /* bSlaveInterface0: Data Class Interface */
    /*Endpoint 2 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    0x82,   /* bEndpointAddress: (IN2) */
    0x03,   /* bmAttributes: Interrupt */
    VIRTUAL_COM_PORT_INT_SIZE,      /* wMaxPacketSize: */
    0x00,
    0xFF,   /* bInterval: */
    /*Data class interface descriptor*/
    0x09,   /* bLength: Endpoint Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
    0x01,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */
    /*Endpoint 3 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    0x03,   /* bEndpointAddress: (OUT3) */
    0x02,   /* bmAttributes: Bulk */
    VIRTUAL_COM_PORT_DATA_SIZE,             /* wMaxPacketSize: */
    0x00,
    0x00,   /* bInterval: ignore for Bulk transfer */
    /*Endpoint 1 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    0x81,   /* bEndpointAddress: (IN1) */
    0x02,   /* bmAttributes: Bulk */
    VIRTUAL_COM_PORT_DATA_SIZE,             /* wMaxPacketSize: */
    0x00,
    0x00    /* bInterval */
  };

/* USB String Descriptors */
const uint8_t Virtual_Com_Port_StringLangID[VIRTUAL_COM_PORT_SIZ_STRING_LANGID] =
  {
    VIRTUAL_COM_PORT_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04 /* LangID = 0x0409: U.S. English */
  };

const uint8_t Virtual_Com_Port_StringVendor[VIRTUAL_COM_PORT_SIZ_STRING_VENDOR] =
  {
    VIRTUAL_COM_PORT_SIZ_STRING_VENDOR,     /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,             /* bDescriptorType*/
    /* Manufacturer: "STMicroelectronics" */
    'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
    'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
    'c', 0, 's', 0
  };

const uint8_t Virtual_Com_Port_StringProduct[VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT] =
  {
    VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    /* Product name: "STM32 Virtual COM Port" */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, ' ', 0, 'V', 0, 'i', 0,
    'r', 0, 't', 0, 'u', 0, 'a', 0, 'l', 0, ' ', 0, 'C', 0, 'O', 0,
    'M', 0, ' ', 0, 'P', 0, 'o', 0, 'r', 0, 't', 0, ' ', 0, ' ', 0
  };

uint8_t Virtual_Com_Port_StringSerial[VIRTUAL_COM_PORT_SIZ_STRING_SERIAL] =
  {
    VIRTUAL_COM_PORT_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0, '1', 0, '0', 0
  }
;
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


///**
//  ******************************************************************************
//  * @file    usb_desc.c
//  * @author  MCD Application Team
//  * @version V3.4.0
//  * @date    09-September-2013
//  * @brief   Descriptors for CUBE Demo
//  ******************************************************************************
//  * @attention
//  *
//  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
//  *
//  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
//  * You may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at:
//  *
//  *        http://www.st.com/software_license_agreement_liberty_v2
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  *
//  ******************************************************************************
//  */
//
//
///* Includes ------------------------------------------------------------------*/
//#include "usb_lib.h"
//#include "usb_desc.h"
//
///* Private typedef -----------------------------------------------------------*/
///* Private define ------------------------------------------------------------*/
///* Private macro -------------------------------------------------------------*/
///* Private variables ---------------------------------------------------------*/
///* Extern variables ----------------------------------------------------------*/
///* Private function prototypes -----------------------------------------------*/
///* Private functions ---------------------------------------------------------*/
//
///* USB Standard Device Descriptor */
//const uint8_t CUBE_DeviceDescriptor[CUBE_SIZ_DEVICE_DESC] =
//  {
//		    CUBE_SIZ_DEVICE_DESC,         // = 18 общая длина дескриптора устройства в байтах
//		    USB_DEVICE_DESCRIPTOR_TYPE, // bDescriptorType - показывает, что это за дескриптор. В данном случае - Device descriptor
//		    0x00, 0x02,                 // bcdUSB - какую версию стандарта USB поддерживает устройство. 2.0
//
//			// класс, подкласс устройства и протокол, по стандарту USB. У нас нули, означает каждый интерфейс сам за себя
//		    0x00,                       //bDeviceClass
//		    0x00,                       //bDeviceSubClass
//		    0x00,                       //bDeviceProtocol
//
//		    0x40,                       //bMaxPacketSize - максимальный размер пакетов для Endpoint 0 (при конфигурировании)
//
//			// те самые пресловутые VID и PID,  по которым и определяется, что же это за устройство.
//			// в реальных устройствах надо покупать VID, чтобы устройства можно было различать и подсовывать нужные драйвера
//		    0x83, 0x04,                 //idVendor (0x0483)
//		    0x11, 0x57,                 //idProduct (0x5711)
//
//		    0x00, 0x01,                 // bcdDevice rel. DEVICE_VER_H.DEVICE_VER_L  номер релиза устройства
//
//			// дальше идут индексы строк, описывающих производителя, устройство и серийный номер.
//			// Отображаются в свойствах устройства в диспетчере устройств
//			// А по серийному номеру подключенные устройства с одинаковым VID/PID различаются системой.
//		    1,                          //Index of string descriptor describing manufacturer
//		    2,                          //Index of string descriptor describing product
//		    3,                          //Index of string descriptor describing the device serial number
//		    0x01                        // bNumConfigurations - количество возможных конфигураций. У нас одна.
//  }
//  ; /* CustomHID_DeviceDescriptor */
//
//
///* USB Configuration Descriptor */
///*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
//const uint8_t CUBE_ConfigDescriptor[CUBE_SIZ_CONFIG_DESC] =
//  {
//		    0x09, 			// bLength: длина дескриптора конфигурации
//		    USB_CONFIGURATION_DESCRIPTOR_TYPE, // bDescriptorType: тип дескриптора - конфигурация
//		    CUBE_SIZ_CONFIG_DESC, 0x00, // wTotalLength: общий размер всего дерева под данной конфигурацией в байтах
//
//		    0x01,         // bNumInterfaces: в конфигурации всего один интерфейс
//		    0x01,         // bConfigurationValue: индекс данной конфигурации
//		    0x00,         // iConfiguration: индекс строки, которая описывает эту конфигурацию
//		    0xE0,         // bmAttributes: признак того, что устройство будет питаться от шины USB
//		    0xFA,         // MaxPower 500 mA
//
//				/************** Дескриптор интерфейса ****************/
//				0x09,         // bLength: размер дескриптора интерфейса
//				USB_INTERFACE_DESCRIPTOR_TYPE, // bDescriptorType: тип дескриптора - интерфейс
//				0x00,         // bInterfaceNumber: порядковый номер интерфейса - 0
//				0x00,         // bAlternateSetting: признак альтернативного интерфейса, у нас не используется
///*----------->*/0x02,         // bNumEndpoints - количество эндпоинтов.
//
//				0x03,         // bInterfaceClass: класс интерфеса - HID
//				// если бы мы косили под стандартное устройство, например клавиатуру или мышь, то надо было бы указать правильно класс и подкласс
//				// а так у нас общее HID-устройство
//				0x00,         // bInterfaceSubClass : подкласс интерфейса.
//				0x00,         // nInterfaceProtocol : протокол интерфейса
//
//				0,            // iInterface: индекс строки, описывающей интерфейс
//
//					// теперь отдельный дескриптор для уточнения того, что данный интерфейс - это HID устройство
//					/******************** HID дескриптор ********************/
//					0x09,         // bLength: длина HID-дескриптора
//					HID_DESCRIPTOR_TYPE, // bDescriptorType: тип дескриптора - HID
//					0x01, 0x01,   // bcdHID: номер версии HID 1.1
///*--------------->*/0x33,         // bCountryCode: код страны (если нужен)
///*===============>*/0x01,         // bNumDescriptors: Сколько дальше будет report дескрипторов
//						HID_REPORT_DESCRIPTOR_TYPE,         // bDescriptorType: Тип дескриптора - report
//						CUBE_SIZ_REPORT_DESC,	0x00, // wItemLength: длина report-дескриптора
//
//
//					/******************** дескриптор конечных точек (endpoints) ********************/
//					0x07,          // bLength: длина дескриптора
//					USB_ENDPOINT_DESCRIPTOR_TYPE, // тип дескриптора - endpoints
//
//					0x81,          // bEndpointAddress: адрес конечной точки и направление 1(IN)
//					0x03,          // bmAttributes: тип конечной точки - Interrupt endpoint
//					wMaxPacketSize, 0x00,    // wMaxPacketSize:  Bytes max
//					0x20,          // bInterval: Polling Interval (32 ms)=0x20
///*==^==^==^==^==^==^==^==^==^==^==^==^*/
//          0x07,	/* bLength: Endpoint Descriptor size */
//          USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
//            /*	Endpoint descriptor type */
//          0x01,	/* bEndpointAddress: */
//            /*	Endpoint Address (OUT) */
//          0x03,	/* bmAttributes: Interrupt endpoint */
//          wMaxPacketSize,	/* wMaxPacketSize:  Bytes max  */
//          0x00,
//          0x20,	/* bInterval: Polling Interval (32 ms) */
//}
//  ; /* CUBE_ConfigDescriptor */
//
//const uint8_t CUBE_ReportDescriptor[CUBE_SIZ_REPORT_DESC] =
//  {
//    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
//    0x09, 0x01,                    // USAGE (Vendor Usage 1)
//    0xa1, 0x01,                    // COLLECTION (Application)
//
////REPORT ID 1 - PC sends first CUBE/2
//0x85, 0x01,                    //REPORT_ID (1)
//0x09, 0x01,                    //USAGE (Vendor Usage 1)
//    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
//0x75, 0x08,                    //   REPORT_SIZE (8)
//    0x95, RPT1_COUNT,                    //REPORT_COUNT (N)
//0xb1, 0x82,                    //   FEATURE (Data,Var,Abs,Vol)
//0x85, 0x01,                    //REPORT_ID (1)
//0x09, 0x01,                    //USAGE (Vendor Usage 1)
//    0x91, 0x82,                    //   OUTPUT (Data,Var,Abs,Vol)
//
////REPORT ID 2 - PC sends second CUBE/2
//0x85, 0x02,                    //REPORT_ID (2)
//0x09, 0x02,                    //USAGE (Vendor Usage 2)
//    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
//0x75, 0x08,                    //   REPORT_SIZE (8)
//    0x95, RPT2_COUNT,                    //REPORT_COUNT (N)
//0xb1, 0x82,                    //   FEATURE (Data,Var,Abs,Vol)
//0x85, 0x02,                    //REPORT_ID (2)
//0x09, 0x02,                    //USAGE (Vendor Usage 2)
//    0x91, 0x82,                    //   OUTPUT (Data,Var,Abs,Vol)
//
////REPORT ID 3 - PC sends ???
//0x85, 0x03,                    //REPORT_ID (3)
//0x09, 0x03,                    //USAGE (Vendor Usage 3)
//    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
//    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
//0x75, 0x08,                    //   REPORT_SIZE (8)
//    0x95, RPT3_COUNT,              //REPORT_COUNT (N)
//0xb1, 0x82,                    //   FEATURE (Data,Var,Abs,Vol)
//0x85, 0x03,                    //REPORT_ID (3)
//0x09, 0x03,                    //USAGE (Vendor Usage 3)
//    0x91, 0x82,                    //   OUTPUT (Data,Var,Abs,Vol)
//
//
////REPORT ID 4 - STM32 sends CUBE/2
//0x85, 0x04,                    //REPORT_ID (4)
//0x09, 0x04,                    //USAGE (Vendor Usage 4)
//    0x75, 0x08,                    //   REPORT_SIZE (8)
//    0x95, RPT4_COUNT,              //REPORT_COUNT (N)
//    0x81, 0x82,                    //   INPUT (Data,Var,Abs,Vol)
//
////REPORT ID 5 - STM32 sends CUBE/2
//0x85, 0x05,                    //REPORT_ID (5)
//0x09, 0x05,                    //USAGE (Vendor Usage 5)
//	0x75, 0x08,                    //   REPORT_SIZE (8)
//	0x95, RPT5_COUNT,              //REPORT_COUNT (N)
//	0x81, 0x82,                    //   INPUT (Data,Var,Abs,Vol)
//
////REPORT ID 6 - STM32 sends buttons + info
//0x85, 0x06,                    //REPORT_ID (6)
//0x09, 0x06,                    //USAGE (Vendor Usage 6)
//	0x75, 0x08,                    //   REPORT_SIZE (8)
//	0x95, RPT6_COUNT,              //REPORT_COUNT (N)
//	0x81, 0x82,                    //   INPUT (Data,Var,Abs,Vol)
//
//    0xc0                           // END_COLLECTION
//}
//  ; /* CUBE_ReportDescriptor */
//
///* USB String Descriptors (optional) */
//const uint8_t CUBE_StringLangID[CUBE_SIZ_STRING_LANGID] =
//  {
//    CUBE_SIZ_STRING_LANGID, /* =4*/
//    USB_STRING_DESCRIPTOR_TYPE,
//    0x09,
//    0x04
//  }
//  ; /* LangID = 0x0409: U.S. English */
//
//const uint8_t CUBE_StringVendor[CUBE_SIZ_STRING_VENDOR] =
//  {
//    CUBE_SIZ_STRING_VENDOR, /* Size of Vendor string = 10*/
//    USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
//    /* Manufacturer: "STMicroelectronics" */
//    'E', 0, '.', 0, 'D', 0, '.', 0
//  };
//
//const uint8_t CUBE_StringProduct[CUBE_SIZ_STRING_PRODUCT] =
//  {
//    CUBE_SIZ_STRING_PRODUCT,          /* bLength = 22*/
//    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
//    'C', 0, 'U', 0, 'B', 0, 'E', 0, ' ', 0, 'S', 0, 'T', 0,
//    'M', 0, '3', 0, '2', 0
//  };
//uint8_t CUBE_StringSerial[CUBE_SIZ_STRING_SERIAL] =
//  {
//    CUBE_SIZ_STRING_SERIAL,           /* bLength = 26 */
//    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
//    'U', 0, 'S', 0, 'B', 0, ' ', 0, 'C', 0, 'U', 0, 'B', 0, 'E'
//  };
//
///************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
//
