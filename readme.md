# 新しいセンサー(Hoge)の追加方法

## I2Cの場合

### Hoge.hを作成
```cpp
#ifndef	__INC_HOGE_H__
#define	__INC_HOGE_H__

#include  "SenseBuffer.h"
#include  "SensorInfo.h"
#include  "I2C.h"

class Hoge:public SensorInfo{
  public:
    virtual void  initialize(void);
    virtual void  get(SenseBuffer buff);
    virtual size_t  build(char *p, SenseBuffer buff);
    virtual void  stop(int sec) {};
    virtual void  start(void) {};
    virtual const char  *name(void);
    virtual const char  *unit(void);
    virtual const char  *data_class_name(void);
    virtual int dimension(void) { return 1; };

    static	const	uint8_t	precision = 0; // センサーデータ送信時の小数点以下の桁数
    esp_err_t	err_code;
    virtual	int	get_ledno(void) { return 1; };  // 点灯するLEDの番号(0～3)

    Hoge();
    ~Hoge();
  protected:
    static	const	int SLA_Hoge = 0x31; // I2CのID
    I2C	mI2C = I2C(SLA_Hoge); // I2Cクラスを作成

    static	const	byte	HOGE_START = 0x52;  // 動作チェック用のレジスタアドレスを指定
    static	const	byte	HOGE_GET_REG = 0x55;  // データ取得用のレジスタアドレス
};
#endif
```

### Hoge.cppを作成
```cpp
#define	TRACE
extern "C" {
#include  "esp_log.h"
}
#include  "config.h"
#include  "debug.h"
#include  "utility.h"
#include  "Hoge.h" // ヘッダーファイル
#define TAG "Hoge"  // ログ出力時のタグ

Hoge::Hoge(){
  initialize();
}

Hoge::~Hoge(){
}

void Hoge::initialize(void){  // 初期化
ENTER_FUNC;
  byte getvalid = mI2C.readByte(HOGE_START);  // 動作チェック用のレジスタアドレスを指定
  err_code = mI2C.err_code;

  ESP_LOGI(TAG, "HOGE_START:%02x", getvalid);

  if(err_code == ESP_OK && getvalid == 0x00){  // 動作チェック用の正しい値を指定
    fValid = true;
    ESP_LOGI(TAG, "valid");
	}else{
    fValid = false;
    ESP_LOGE(TAG, "invalid");
  }
LEAVE_FUNC;
}

void Hoge::get(SenseBuffer buff){ // センサーデータ取得
ENTER_FUNC;
  byte	tbuff[2]; // センサーデータ格納用変数
  int piyo;

  mI2C.read(HOGE_GET_REG, tbuff, 2); // mI2C.read(データ取得用のレジスタアドレス, センサーデータ格納用変数のアドレス, 取得するbyte数)
  err_code = mI2C.err_code;  // I2Cエラーコード取得

  if (err_code == ESP_OK) {  // I2Cがエラーでない場合
    piyo = tbuff[0] << 8 | tbuff[1]; // センサーデータの編集
    ESP_LOGI(TAG, "piyo = %d", piyo);
  } else {  // I2Cがエラーの場合
    piyo = INT_MAX;   // 最大値を設定
    ESP_LOGE(TAG, "err_code:%d", err_code);
  }
  buff.put(&piyo, sizeof(int)); // センサーデータ収集バッファに編集後の値を設定
LEAVE_FUNC;
}

size_t Hoge::build(char *p, SenseBuffer buff){  // センサーデータをfolloger形式の文字列に編集
ENTER_FUNC;
  int	data;
  buff.get(&data, sizeof(int));  // バッファからセンサーデータを取得
LEAVE_FUNC;
  return	sprintf(p, "\"value\":\"%d\",\"unit\":\"%s\"", data, unit()); // センサーデータに該当するfolloger形式の文字列に編集
}

const char* Hoge::name(void){
  return	"Hoge"; // センサー名
}

const char* Hoge::unit(void){
  return	"ppm";  // センサーデータに該当する単位
}

const char* Hoge::data_class_name(void){
  return	"CO2Concentration"; // センサーデータに該当するfollogerのdata_class_name
}
```

## UARTの場合

### Hoge.hを作成
```cpp
#ifndef	__INC_HOGE_H__
#define	__INC_HOGE_H__

#include	"SenseBuffer.h"
#include	"SensorInfo.h"
#include	"UART.h"

class Hoge:public SensorInfo
{
  public:
    virtual void	initialize(void);
    virtual	void	get(SenseBuffer buff);
    virtual	size_t	build(char *p, SenseBuffer buff);
    virtual	void	stop(int sec){};
    virtual	void	start(void){};
    virtual	const	char	*name(void);
    virtual	const	char	*unit(void);
    virtual	const	char	*data_class_name(void);
    virtual	int	dimension(void) { return 1; };

    static	const	uint8_t	precision = 0; // センサーデータ送信時の小数点以下の桁数
    esp_err_t	err_code;
    virtual	int	get_ledno(void) { return 1; };  // 点灯するLEDの番号(0～3)

    Hoge();
    ~Hoge();
};

#endif
```

### Hoge.cppを作成
```cpp
#define	TRACE
extern "C" {
#include  "driver/uart.h"
#include  "esp_log.h"
}
#include  "config.h"
#include  "debug.h"
#include  "utility.h"
#include  "Hoge.h" // ヘッダーファイル
#define TAG "Hoge"  // ログ出力時のタグ

LuminOx::LuminOx(){
  initialize();
}

LuminOx::~LuminOx(){
}

void　LuminOx::initialize(void){
ENTER_FUNC;

  char *line = readLine(); // 動作チェックデータ取得
  char e[7];
  strncpy(e, strchr(line, 'e'), 6);
  e[6] = '\0';
  ESP_LOGI(TAG, "LuminOx_e:%s", e);
  if(strcmp(e,"e 0000") == 0){  // 動作チェック用の正しい値を指定
    fValid = true;
    ESP_LOGI(TAG, "valid");
  }else{
    fValid = false;
    ESP_LOGE(TAG, "invalid");
  }

LEAVE_FUNC;
}

void LuminOx::get(SenseBuffer	buff){  // センサーデータ取得
ENTER_FUNC;
  uart_write_bytes(UART_NUM_2, "%\r\n", 3);  // データ取得コマンドを送信
  char *line = readLine(UART_NUM_2);  // データ取得コマンドの結果を受信
  char p[7];  // データ編集
  float per;
  strncpy(p, strchr(line, '%') + 2, 6);
  p[6] = '\0';
  per = atof(p);
  ESP_LOGI(TAG, "O2 = %s", p);
  buff.put(&per, sizeof(float));  // センサーデータ収集バッファに編集後の値を設定
LEAVE_FUNC;
}

size_t LuminOx::build(char	*p,SenseBuffer	buff){  // センサーデータをfolloger形式の文字列に編集
ENTER_FUNC;
  float per;
  buff.get(&per, sizeof(float));  // バッファからセンサーデータを取得
LEAVE_FUNC;
  return	sprintf(p, "\"value\":\"%.2f\",\"unit\":\"%s\"",per,"%");// センサーデータに該当するfolloger形式の文字列に編集
}

const char* Hoge::name(void){
  return	"Hoge"; // センサー名
}

const char* Hoge::unit(void){
  return	"%";  // センサーデータに該当する単位
}

const char* Hoge::data_class_name(void){
  return	"O2Concentration"; // センサーデータに該当するfollogerのdata_class_name
}
```

## SensorFarm.cppを編集

```cpp
～省略～

/* sensor class header */
#include  "S300.h"
#include  "BME280_Barometric.h"
#include  "BME280_Humidity.h"
#include  "BME280_Temp.h"
#include  "MPU9250_Accelerometer.h"
#include  "MPU9250_Gyroscope.h"
#include  "MPU9250_Magnetometer.h"
#include  "LuminOx.h"

#include  "Hoge.h"  // ヘッダーファイルを追加

#define TAG	"SensorFarm"

～省略～

void main_task(void	*para){
ENTER_FUNC;
  Sensors::init();
  CAT9554 cat9554;
  if(!cat9554.init_control()){
    task_stop();
  }
  byte id = cat9554.getID();
  if(cat9554.err_code != ESP_OK){
    task_stop();
  }

  /* Gas Sensor Board */
  if(id == 0x00){
  ESP_LOGI(TAG, "Gas Sensor Board start");

  Sensors::add((SensorInfo*)new S300());
  Sensors::add((SensorInfo*)new BME280_Barometric());
  Sensors::add((SensorInfo*)new BME280_Humidity());
  Sensors::add((SensorInfo*)new BME280_Temp());
  Sensors::add((SensorInfo*)new MPU9250_Accelerometer());
  Sensors::add((SensorInfo*)new MPU9250_Gyroscope());
  Sensors::add((SensorInfo*)new MPU9250_Magnetometer());
  Sensors::add((SensorInfo*)new LuminOx());

  /* Voltage Sensor Board */
  }else if(id == 0x01){
    ESP_LOGI(TAG, "Voltage Sensor Board start");

  /* HogeHoge Board */  // 新しいセンサー基板を追加する場合
  }else if(id == 0x02){ // IOエキスパンダに設定したボードIDを設定
    ESP_LOGI(TAG, "HogeHoge Sensor Board start");

    Sensors::add((SensorInfo*)new Hoge()); // センサークラスを追加

  /* Unknown Board */
  }else{
    ESP_LOGE(TAG, "Unknown sensor board. id:%02x", id);
    task_stop();
  }

～省略～

```

