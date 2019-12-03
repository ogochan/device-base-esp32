#define	TRACE

#include 	<stdio.h>
#include	<ctype.h>

#include 	"config.h"
#include 	"freertos/FreeRTOS.h"
#include 	"freertos/task.h"
#include	"freertos/event_groups.h"
#include 	"esp_system.h"
#include	"esp_wifi.h"
#include	"esp_event_loop.h"
#include	"esp_bt.h"
#include	"esp_bt_main.h"
#include	"esp_bt_device.h"
#include	"esp_spp_api.h"
#include	"esp_gap_bt_api.h"
#include 	"esp_log.h"
#include	"time.h"
#include	<sys/time.h>
#include	<string.h>

#include	"bt.h"
#include	"types.h"
#include	"misc.h"
#include 	"debug.h"


const char * _spp_server_name = "ESP32SPP";

//static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

//static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
//static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

static	uint32_t	handle;

#define	SPP_TX_MAX	330

static uint8_t	_spp_tx_buffer[SPP_TX_MAX];

static uint16_t _spp_tx_buffer_len = 0;

#define RX_QUEUE_SIZE 512
#define TX_QUEUE_SIZE 32
static uint32_t _spp_client = 0;
static xQueueHandle _spp_rx_queue = NULL;
static xQueueHandle _spp_tx_queue = NULL;
static SemaphoreHandle_t _spp_tx_done = NULL;
static TaskHandle_t _spp_task_handle = NULL;
static EventGroupHandle_t _spp_event_group = NULL;
static Bool	secondConnectionAttempt;
static esp_spp_cb_t * custom_spp_callback = NULL;

#define SPP_RUNNING     0x01
#define SPP_CONNECTED   0x02
#define SPP_CONGESTED   0x04

typedef struct {
        size_t len;
        uint8_t data[];
} spp_packet_t;

static esp_err_t _spp_queue_packet(uint8_t *data, size_t len){
    if(!data || !len){
        dbgmsg("No data provided");
        return ESP_OK;
    }
    spp_packet_t * packet = (spp_packet_t*)malloc(sizeof(spp_packet_t) + len);
    if(!packet){
        dbgmsg("SPP TX Packet Malloc Failed!");
        return ESP_FAIL;
    }
    packet->len = len;
    memcpy(packet->data, data, len);
    if (xQueueSend(_spp_tx_queue, &packet, portMAX_DELAY) != pdPASS) {
        dbgmsg("SPP TX Queue Send Failed!");
        free(packet);
        return ESP_FAIL;
    }
    return ESP_OK;
}

static	Bool
btStarted(void){
    return (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED);
}

static	Bool
btStart(void){
    esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED){
        return true;
    }
    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE){
        esp_bt_controller_init(&cfg);
        while(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE){}
    }
    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED){
        if (esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) {
            dbgmsg("BT Enable failed");
            return false;
        }
    }
    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED){
        return true;
    }
    dbgmsg("BT Start failed");
    return false;
}

static	bool
btStop(void){
    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE){
        return true;
    }
    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED){
        if (esp_bt_controller_disable()) {
            dbgmsg("BT Disable failed");
            return false;
        }
        while(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED);
    }
    if(esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED){
        return true;
    }
    dbgmsg("BT Stop failed");
    return false;
}

static bool
_spp_send_buffer(){
    if((xEventGroupWaitBits(_spp_event_group, SPP_CONGESTED, pdFALSE, pdTRUE, portMAX_DELAY) & SPP_CONGESTED)){
        esp_err_t err = esp_spp_write(_spp_client, _spp_tx_buffer_len, _spp_tx_buffer);
        if(err != ESP_OK){
            dbgprintf("SPP Write Failed! [0x%X]", err);
            return false;
        }
        _spp_tx_buffer_len = 0;
        if(xSemaphoreTake(_spp_tx_done, portMAX_DELAY) != pdTRUE){
            dbgmsg("SPP Ack Failed!");
            return false;
        }
        return true;
    }
    return false;
}

static void
_spp_tx_task(void * arg){
    spp_packet_t *packet = NULL;
    size_t len = 0, to_send = 0;
    uint8_t * data = NULL;
    for (;;) {
        if(_spp_tx_queue && xQueueReceive(_spp_tx_queue, &packet, portMAX_DELAY) == pdTRUE && packet){
            if(packet->len <= (SPP_TX_MAX - _spp_tx_buffer_len)){
                memcpy(_spp_tx_buffer+_spp_tx_buffer_len, packet->data, packet->len);
                _spp_tx_buffer_len+=packet->len;
                free(packet);
                packet = NULL;
                if(SPP_TX_MAX == _spp_tx_buffer_len || uxQueueMessagesWaiting(_spp_tx_queue) == 0){
                    _spp_send_buffer();
                }
            } else {
                len = packet->len;
                data = packet->data;
                to_send = SPP_TX_MAX - _spp_tx_buffer_len;
                memcpy(_spp_tx_buffer+_spp_tx_buffer_len, data, to_send);
                _spp_tx_buffer_len = SPP_TX_MAX;
                data += to_send;
                len -= to_send;
                _spp_send_buffer();
                while(len >= SPP_TX_MAX){
                    memcpy(_spp_tx_buffer, data, SPP_TX_MAX);
                    _spp_tx_buffer_len = SPP_TX_MAX;
                    data += SPP_TX_MAX;
                    len -= SPP_TX_MAX;
                    _spp_send_buffer();
                }
                if(len){
                    memcpy(_spp_tx_buffer, data, len);
                    _spp_tx_buffer_len += len;
                    free(packet);
                    packet = NULL;
                    if(uxQueueMessagesWaiting(_spp_tx_queue) == 0){
                        _spp_send_buffer();
                    }
                }
            }
        } else {
            dbgmsg("Something went horribly wrong");
        }
    }
    vTaskDelete(NULL);
    _spp_task_handle = NULL;
}

static void
esp_spp_cb(
	esp_spp_cb_event_t	event,
	esp_spp_cb_param_t	*param)
{
    switch (event)	{
	  case ESP_SPP_INIT_EVT:
		dbgmsg("ESP_SPP_INIT_EVT");
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, _spp_server_name);
        xEventGroupSetBits(_spp_event_group, SPP_RUNNING);
        break;

	  case ESP_SPP_SRV_OPEN_EVT://Server connection open
        if (!_spp_client){
            _spp_client = param->open.handle;
        } else {
            secondConnectionAttempt = true;
            esp_spp_disconnect(param->open.handle);
        }
        xEventGroupSetBits(_spp_event_group, SPP_CONNECTED);
        dbgmsg("ESP_SPP_SRV_OPEN_EVT");
        break;

	  case ESP_SPP_CLOSE_EVT://Client connection closed
        if(secondConnectionAttempt) {
            secondConnectionAttempt = false;
        } else {
            _spp_client = 0;
        }
        xEventGroupClearBits(_spp_event_group, SPP_CONNECTED);
        dbgmsg("ESP_SPP_CLOSE_EVT");
        break;

	  case ESP_SPP_CONG_EVT://connection congestion status changed
        if(param->cong.cong){
            xEventGroupClearBits(_spp_event_group, SPP_CONGESTED);
        } else {
            xEventGroupSetBits(_spp_event_group, SPP_CONGESTED);
        }
        dbgprintf("ESP_SPP_CONG_EVT: %s", param->cong.cong?"CONGESTED":"FREE");
        break;

	  case ESP_SPP_WRITE_EVT://write operation completed
        if(param->write.cong){
            xEventGroupClearBits(_spp_event_group, SPP_CONGESTED);
        }
        xSemaphoreGive(_spp_tx_done);//we can try to send another packet
        dbgprintf("ESP_SPP_WRITE_EVT: %u %s", param->write.len, param->write.cong?"CONGESTED":"FREE");
        break;

	  case ESP_SPP_DATA_IND_EVT://connection received data
        dbgprintf("ESP_SPP_DATA_IND_EVT len=%d handle=%d", param->data_ind.len, param->data_ind.handle);
        //esp_log_buffer_hex("",param->data_ind.data,param->data_ind.len); //for low level debug
        //ets_printf("r:%u\n", param->data_ind.len);

        if (_spp_rx_queue != NULL){
            for (int i = 0; i < param->data_ind.len; i++){
                if(xQueueSend(_spp_rx_queue, param->data_ind.data + i, (TickType_t)0) != pdTRUE){
                    dbgprintf("RX Full! Discarding %u bytes", param->data_ind.len - i);
                    break;
                }
            }
        }
        break;

        //should maybe delete those.
	  case ESP_SPP_DISCOVERY_COMP_EVT://discovery complete
        dbgmsg("ESP_SPP_DISCOVERY_COMP_EVT");
        break;
	  case ESP_SPP_OPEN_EVT://Client connection open
        dbgmsg("ESP_SPP_OPEN_EVT");
        break;
	  case ESP_SPP_START_EVT://server started
        dbgmsg("ESP_SPP_START_EVT");
        break;
	  case ESP_SPP_CL_INIT_EVT://client initiated a connection
        dbgmsg("ESP_SPP_CL_INIT_EVT");
        break;
	  default:
        break;
    }
    if(custom_spp_callback)(*custom_spp_callback)(event, param);
}

extern	int
bt_getc(void)
{
    uint8_t c = 0;

    if (_spp_rx_queue ) {
		while	( !xQueueReceive(_spp_rx_queue, &c, 1000 / portTICK_RATE_MS) );
        return c;
    } else {
		return -1;
	}
}

extern	void
bt_send(
	uint8_t	*data,
	size_t	len)
{
    if (!_spp_client){
        return 0;
    }
    return (_spp_queue_packet(data, len) == ESP_OK) ? len : 0;
}

extern	void
bt_send_string(
	char	*str)
{
	bt_send((uint8_t *)str, strlen(str));
}

extern	size_t
bt_recv(
	uint8_t	*data,
	size_t	len)
{
	size_t	i;
	int		c;

	for	( i = 0 ; i < len ; i ++ )	{
		if	( ( c = bt_getc() ) < 0 )	break;
		*data = c;
		data ++;
	}
	return	(i);
}

extern	size_t
bt_recv_str(
	char	*data,
	int		delim,
	size_t	len)
{
	size_t	i;
	int		c;

	for	( i = 0 ; i < len ; i ++ )	{
		c = bt_getc();
		*data = c;
		data ++;
		if	( c == delim )	break;
	}
	*data = 0;
	return	(i);
}

extern	void
initialize_bt(const char *deviceName)
{
    if(!_spp_event_group){
        _spp_event_group = xEventGroupCreate();
        if(!_spp_event_group){
            dbgmsg("SPP Event Group Create Failed!");
            return false;
        }
        xEventGroupClearBits(_spp_event_group, 0xFFFFFF);
        xEventGroupSetBits(_spp_event_group, SPP_CONGESTED);
    }
    if (_spp_rx_queue == NULL){
        _spp_rx_queue = xQueueCreate(RX_QUEUE_SIZE, sizeof(uint8_t)); //initialize the queue
        if (_spp_rx_queue == NULL){
            dbgmsg("RX Queue Create Failed");
            return false;
        }
    }
    if (_spp_tx_queue == NULL){
        _spp_tx_queue = xQueueCreate(TX_QUEUE_SIZE, sizeof(spp_packet_t*)); //initialize the queue
        if (_spp_tx_queue == NULL){
            dbgmsg("TX Queue Create Failed");
            return false;
        }
    }
    if(_spp_tx_done == NULL){
        _spp_tx_done = xSemaphoreCreateBinary();
        if (_spp_tx_done == NULL){
            dbgmsg("TX Semaphore Create Failed");
            return false;
        }
        xSemaphoreTake(_spp_tx_done, 0);
    }

    if(!_spp_task_handle){
        xTaskCreate(_spp_tx_task, "spp_tx", 4096, NULL, 2, &_spp_task_handle);
        if(!_spp_task_handle){
            dbgmsg("Network Event Task Start Failed!");
            return false;
        }
    }

    if (!btStarted() && !btStart()){
        dbgmsg("initialize controller failed");
        return false;
    }

    esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
    if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED){
        if (esp_bluedroid_init()) {
            dbgmsg("initialize bluedroid failed");
            return false;
        }
    }
    
    if (bt_state != ESP_BLUEDROID_STATUS_ENABLED){
        if (esp_bluedroid_enable()) {
            dbgmsg("enable bluedroid failed");
            return false;
        }
    }

    if (esp_spp_register_callback(esp_spp_cb) != ESP_OK){
        dbgmsg("spp register failed");
        return false;
    }

    if (esp_spp_init(ESP_SPP_MODE_CB) != ESP_OK){
        dbgmsg("spp init failed");
        return false;
    }

    esp_bt_dev_set_device_name(deviceName);

    // the default BTA_DM_COD_LOUDSPEAKER does not work with the macOS BT stack
    esp_bt_cod_t cod;
    cod.major = 0b00001;
    cod.minor = 0b000100;
    cod.service = 0b00000010110;
    if (esp_bt_gap_set_cod(cod, ESP_BT_INIT_COD) != ESP_OK) {
        dbgmsg("set cod failed");
        return false;
    }

    return true;
}

extern bool
stop_bt()
{
    if (btStarted()){
        if(_spp_client)
            esp_spp_disconnect(_spp_client);
        esp_spp_deinit();
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        btStop();
    }
    _spp_client = 0;
    if(_spp_task_handle){
        vTaskDelete(_spp_task_handle);
        _spp_task_handle = NULL;
    }
    if(_spp_event_group){
        vEventGroupDelete(_spp_event_group);
        _spp_event_group = NULL;
    }
    if(_spp_rx_queue){
        vQueueDelete(_spp_rx_queue);
        //ToDo: clear RX queue when in packet mode
        _spp_rx_queue = NULL;
    }
    if(_spp_tx_queue){
        spp_packet_t *packet = NULL;
        while(xQueueReceive(_spp_tx_queue, &packet, 0) == pdTRUE){
            free(packet);
        }
        vQueueDelete(_spp_tx_queue);
        _spp_tx_queue = NULL;
    }
    if (_spp_tx_done) {
        vSemaphoreDelete(_spp_tx_done);
        _spp_tx_done = NULL;
    }
    return true;
}

#define	COMMAND_BUFFER_SIZE		255
#define	BODY_BUFFER_SIZE		1024
static	char	command_buffer[COMMAND_BUFFER_SIZE];
static	char	body_buffer[BODY_BUFFER_SIZE];

#define	NR_BT_HANDLER			16

static	BT_HandlerTable_s	BT_HandlerTable[NR_BT_HANDLER];
static	size_t				nBT_HandlerTable = 0;

extern	void
bt_set_handler(
	int		method,
	char	*path,
	int		(*func)(char *, char *, Bool))
{
	BT_HandlerTable[nBT_HandlerTable].method = method;
	BT_HandlerTable[nBT_HandlerTable].path = path;
	BT_HandlerTable[nBT_HandlerTable].func = func;
	nBT_HandlerTable ++;
}


static	char	*
skip_space(
	char	*p)
{
	while	(	( *p != 0 )
				&&	( isspace((int)*p) ))	{
		p ++;
	}
	return	(p);
}

static	void
bt_task(
	void	*para)
{
	size_t	len;
	char	*p
		,	*path;
	int		method
		,	ret;
	BT_HandlerTable_s	*ent;

ENTER_FUNC;
	while(1){
		len = bt_recv_str(command_buffer, '\n',COMMAND_BUFFER_SIZE);
		command_buffer[len] = 0;
		dbgprintf("recv(%d) [%s]", (int)len, command_buffer);
		if ( !strlcmp(command_buffer, "GET ") )	{
			method = BT_GET;
			p = &command_buffer[4];
			path = skip_space(p);
		} else
		if	( !strlcmp(command_buffer, "POST ") )	{
			method = BT_POST;
			p = &command_buffer[5];
			path = skip_space(p);
			p = body_buffer;
			do	{
				len = bt_recv_str(p, '\n', BODY_BUFFER_SIZE);
				p += len + 1;
			}	while	( len > 0 );
			dbgprintf("body [%s]", body_buffer);
		} else {
			method = 0;
			path = NULL;
		}
		for	( int i = 0 ; i < nBT_HandlerTable ; i ++ )	{
			ent = &BT_HandlerTable[i];
			if	(	( ent->method == method )
				&&	( !strlcmp(ent->path, path) ))	{
				ret = ent->func(path, body_buffer, false);
				break;
			}
		}
		//bt_send_string("Loop!\n");
		//vTaskDelay(10000 / portTICK_RATE_MS);
		dbgmsg("Loop!");
	}
}

extern	void
start_bt(
	size_t	sStack)
{
	xTaskCreate(bt_task, "bt", sStack, NULL, 1, NULL);
}

