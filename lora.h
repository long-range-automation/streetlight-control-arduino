#define TX_INTERVAL 120

#define LORA_PORT 1

#define LORA_NO_CONFIRMATION 0
#define LORA_REQ_CONFIRMATION 1

void lora_send_immediately();
void lora_start();
void lora_once();