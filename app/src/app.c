#include <stdio.h>
#include <ctype.h>
#include <FreeRTOS.h>
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "SEGGER_RTT.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "cli_commands.h"
#include "cli_commands_fs.h"
#include "fatfs.h"
#include "W25Qxx.h"

#define STORAGE_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define USBD_STACK_SIZE    (configMINIMAL_STACK_SIZE * (CFG_TUSB_DEBUG ? 4 : 2))
#define CDC_STACK_SIZE      (configMINIMAL_STACK_SIZE * (CFG_TUSB_DEBUG ? 3 : 2))
#define BLINKY_STACK_SIZE   configMINIMAL_STACK_SIZE

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
//--------------------------------------------------------------------+

#define URL  "example.tinyusb.org/webusb-serial/index.html"

const tusb_desc_webusb_url_t desc_url = {
  .bLength         = 3 + sizeof(URL) - 1,
  .bDescriptorType = 3, // WEBUSB URL type
  .bScheme         = 1, // 0: http, 1: https
  .url             = URL
};

static bool web_serial_connected = false;

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

static void usb_device_task(void *param);
void led_blinking_task(void* param);
void cdc_task(void *params);

void msc_disk_init(void);

void init(void){
	SEGGER_RTT_ConfigUpBuffer( 0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM );
	SEGGER_RTT_WriteString( 0, "SEGGER Real-Time-Terminal Started\n" );
}

void setup(void){
	uint32_t ID = W25Q_ReadID();
	printf("ID = 0x%08lX\r\n", ID);

	FATFS_Init();
	xTaskCreate(led_blinking_task, "blinky", BLINKY_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
	xTaskCreate(cdc_task, "cdc", CDC_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);
	vTaskStartScheduler();
}

// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
static void usb_device_task(void *param) {
  (void) param;

  // init device stack on configured roothub port
  // This should be called after scheduler/kernel is started.
  // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
  // init device stack on configured roothub port
    // This should be called after scheduler/kernel is started.
    // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
  tud_init(BOARD_TUD_RHPORT);

  msc_disk_init();
  // RTOS forever loop
  while (1) {
    // put this thread to waiting state until there is new events
    tud_task();

    // following code only run if tud_task() process at least 1 event
    tud_cdc_write_flush();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
  printf("USB device mounted\n\r");
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  blink_interval_ms = BLINK_NOT_MOUNTED;
  printf("USB device unmounted\n\r");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
  printf("USB bus suspended\n\r");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
  printf("USB bus resumed\n\r");
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
SemaphoreHandle_t cdc_tx_sem;
QueueHandle_t	  cdc_rx_queue;

#define USB_PACKET_SIZE	64

void tud_cdc_send(uint8_t *buffer, uint32_t bufsize, TickType_t timeout){
	if (bufsize <= USB_PACKET_SIZE){
		tud_cdc_write((uint8_t *)buffer, bufsize);
	    tud_cdc_write_flush();
	    xSemaphoreTake(cdc_tx_sem, timeout);
	}else{
		uint32_t len = 0;
		while(bufsize){
			if (bufsize > USB_PACKET_SIZE){
				len = USB_PACKET_SIZE;
			}else{
				len = bufsize;
			}
			tud_cdc_write((uint8_t *)buffer, len);
			tud_cdc_write_flush();
			xSemaphoreTake(cdc_tx_sem, timeout);
			buffer += len;
			bufsize -= len;
		}
	}
}

uint32_t tud_cdc_receive(uint8_t *buffer, uint32_t bufsize, TickType_t timeout){
	uint32_t len;
	xQueueReceive(cdc_rx_queue, &len, timeout);
	if (len > bufsize){
		len = bufsize;
	}
	uint32_t count = tud_cdc_read(buffer, len);
	return count;
}


void print_string(char *string, TickType_t timeout) {
	tud_cdc_send((uint8_t *)string, strlen(string), timeout);
}

void print_char(char character, TickType_t timeout) {
	tud_cdc_send((uint8_t *)&character, 1, timeout);
}

#define MAX_INPUT_LENGTH    32
#define MAX_OUTPUT_LENGTH   512

void cdc_task(void *params) {
	(void) params;
	char cRxedChar;
	BaseType_t cInputIndex = 0;
	BaseType_t xMoreDataToFollow;
	/* The input and output buffers are declared static to keep them off the stack. */
	static char pcOutputString[ MAX_OUTPUT_LENGTH ], pcInputString[ MAX_INPUT_LENGTH ];

	CLI_install_commands();
	CLI_install_commands_fs();
	cdc_rx_queue = xQueueCreate(8, sizeof(uint32_t));
	cdc_tx_sem = xSemaphoreCreateBinary();

	do {
		vTaskDelay(10);
	}while (!tud_cdc_connected());

	print_string("Welcome to FreeRTOS ", portMAX_DELAY);
	print_string(tskKERNEL_VERSION_NUMBER, portMAX_DELAY);
	print_string("\n\r", portMAX_DELAY);
	print_string(">>", portMAX_DELAY);

	// RTOS forever loop
	while(1){
		/* This implementation reads a single character at a time.  Wait in the
		Blocked state until a character is received. */
		(void)tud_cdc_receive((uint8_t *)&cRxedChar, 1, portMAX_DELAY);

		if( cRxedChar == '\r' )
		{
			/* A newline character was received, so the input command string is
			complete and can be processed.  Transmit a line separator, just to
			make the output easier to read. */
			print_string("\n\r", portMAX_DELAY);

			/* The command interpreter is called repeatedly until it returns
			pdFALSE.  See the "Implementing a command" documentation for an
			exaplanation of why this is. */
			do
			{
				/* Send the command string to the command interpreter.  Any
				output generated by the command interpreter will be placed in the
				pcOutputString buffer. */
				xMoreDataToFollow = FreeRTOS_CLIProcessCommand
							  (
								  pcInputString,   /* The command string.*/
								  pcOutputString,  /* The output buffer. */
								  MAX_OUTPUT_LENGTH/* The size of the output buffer. */
							  );

				/* Write the output generated by the command interpreter to the
				console. */
				print_string(pcOutputString, portMAX_DELAY);
				memset(pcOutputString, 0, MAX_OUTPUT_LENGTH);

			} while( xMoreDataToFollow != pdFALSE );
			print_string(">>", portMAX_DELAY);

			/* All the strings generated by the input command have been sent.
			Processing of the command is complete.  Clear the input string ready
			to receive the next command. */
			cInputIndex = 0;
			memset( pcInputString, 0x00, MAX_INPUT_LENGTH );
		}
		else
		{
			/* The if() clause performs the processing after a newline character
			is received.  This else clause performs the processing if any other
			character is received. */

			if( cRxedChar == '\n' )
			{
				/* Ignore carriage returns. */
			}
			//else if( cRxedChar == '\b' )
			else if( cRxedChar == 0x7F )
			{
				/* Backspace was pressed.  Erase the last character in the input
				buffer - if there are any. */
				if( cInputIndex > 0 )
				{
					cInputIndex--;
					pcInputString[ cInputIndex ] = (char)'\0';
					print_char(cRxedChar, portMAX_DELAY);
				}
			}
			else
			{
				/* A character was entered.  It was not a new line, backspace
				or carriage return, so it is accepted as part of the input and
				placed into the input buffer.  When a \n is entered the complete
				string will be passed to the command interpreter. */
				if( cInputIndex < MAX_INPUT_LENGTH )
				{
					pcInputString[ cInputIndex ] = cRxedChar;
					cInputIndex++;
				}
				print_char(cRxedChar, portMAX_DELAY);
			}
		}
	}
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) {
  (void) itf;
  (void) rts;

  if (dtr) {
    printf("Terminal connected\n\r");
  } else {
    printf("Terminal disconnected\n\r");
  }
}


void tud_cdc_tx_complete_cb(uint8_t itf) {
	  (void) itf;
	  xSemaphoreGive(cdc_tx_sem);
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf) {
	portBASE_TYPE high_priority_task_woken = pdFALSE;
	uint32_t len = tud_cdc_n_available(itf);
	xQueueSendToBackFromISR(cdc_rx_queue, &len, &high_priority_task_woken);
	portYIELD_FROM_ISR(high_priority_task_woken);
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void* param) {
  (void) param;
    static bool led_state = false;

  while (1) {
    // Blink every interval ms
    vTaskDelay(blink_interval_ms / portTICK_PERIOD_MS);
    //board_led_write(led_state);
    led_state = 1 - led_state; // toggle
	static uint32_t i;
	printf("blink %04d\n\r", i++);
  }
}

size_t board_get_unique_id(uint8_t id[], size_t max_len) {
  (void) max_len;
  volatile uint32_t *stm32_uuid = (volatile uint32_t *) UID_BASE;
  uint32_t *id32 = (uint32_t *) (uintptr_t) id;
  uint8_t const len = 12;

  id32[0] = stm32_uuid[0];
  id32[1] = stm32_uuid[1];
  id32[2] = stm32_uuid[2];

  return len;
}

//--------------------------------------------------------------------+
// WebUSB use vendor class
//--------------------------------------------------------------------+

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage (setup/data/ack)
// return false to stall control endpoint (e.g unsupported request)
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const* request) {
  // nothing to with DATA & ACK stage
  if (stage != CONTROL_STAGE_SETUP) return true;

  switch (request->bmRequestType_bit.type) {
    case TUSB_REQ_TYPE_VENDOR:
      switch (request->bRequest) {
        case VENDOR_REQUEST_WEBUSB:
          // match vendor request in BOS descriptor
          // Get landing page url
          return tud_control_xfer(rhport, request, (void*)(uintptr_t)&desc_url, desc_url.bLength);

        case VENDOR_REQUEST_MICROSOFT:
          if (request->wIndex == 7) {
            // Get Microsoft OS 2.0 compatible descriptor
            uint16_t total_len;
            memcpy(&total_len, desc_ms_os_20 + 8, 2);

            return tud_control_xfer(rhport, request, (void*)(uintptr_t)desc_ms_os_20, total_len);
          } else {
            return false;
          }

        default: break;
      }
      break;

    case TUSB_REQ_TYPE_CLASS:
      if (request->bRequest == 0x22) {
        // Webserial simulate the CDC_REQUEST_SET_CONTROL_LINE_STATE (0x22) to connect and disconnect.
        web_serial_connected = (request->wValue != 0);

        // Always lit LED if connected
        if (web_serial_connected) {
          //board_led_write(true);
          //blink_interval_ms = BLINK_ALWAYS_ON;

          tud_vendor_write_str("\r\nWebUSB interface connected\r\n");
          tud_vendor_write_flush();
        } else {
          blink_interval_ms = BLINK_MOUNTED;
        }

        // response with status OK
        return tud_control_status(rhport, request);
      }
      break;

    default: break;
  }

  // stall unknown request
  return false;
}

void tud_vendor_rx_cb(uint8_t itf, uint8_t const* buffer, uint16_t bufsize) {
  (void) itf;

 // echo_all(buffer, bufsize);

  // if using RX buffered is enabled, we need to flush the buffer to make room for new data
  #if CFG_TUD_VENDOR_RX_BUFSIZE > 0
  tud_vendor_read_flush();
  #endif
}