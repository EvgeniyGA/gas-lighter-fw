#include <FreeRTOS.h>
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "SEGGER_RTT.h"
#include "tusb.h"
#include <ctype.h>
#include "usb_descriptors.h"
#include <stdio.h>
#include "cli_commands.h"

#define USBD_STACK_SIZE    (configMINIMAL_STACK_SIZE * (CFG_TUSB_DEBUG ? 4 : 2))
#define CDC_STACK_SIZE      (configMINIMAL_STACK_SIZE * (CFG_TUSB_DEBUG ? 3 : 2))
#define BLINKY_STACK_SIZE   configMINIMAL_STACK_SIZE

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
//--------------------------------------------------------------------+

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

void init(void){
	SEGGER_RTT_ConfigUpBuffer( 0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM );
  SEGGER_RTT_WriteString( 0, "SEGGER Real-Time-Terminal Started\n" );
}

void setup(void){
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
  printf("USB device mounter\n\r");
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
  }
}

