#include <projecto001_inferencing.h>

/* Edge Impulse Arduino examples
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * This code was adapted by Marcelo Rovai to run on a XIAO ESP32S3
 * 29May23
 * 
 */

// If your target is limited in memory remove this macro to save 10K RAM
 
/*
 ** NOTE: If you run into TFLite arena allocation issue.
 **
 ** This may be due to may dynamic memory fragmentation.
 ** Try defining "-DEI_CLASSIFIER_ALLOCATION_STATIC" in boards.local.txt (create
 ** if it doesn't exist) and copy this file to
 ** `<ARDUINO_CORE_INSTALL_PATH>/arduino/hardware/<mbed_core>/<core_version>/`.
 **
 ** See
 ** (https://support.arduino.cc/hc/en-us/articles/360012076960-Where-are-the-installed-cores-located-)
 ** to find where Arduino installs cores on your machine.
 **
 ** If the problem persists then there's not enough memory for this model and application.
 */

/* Includes ---------------------------------------------------------------- */

#include <I2S.h>
#define SAMPLE_RATE 16000U
#define SAMPLE_BITS 16
#define LED_BUILT_IN 21 
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>  // Include the ArduinoJson library

const char* ssid = "rayyan";
const char* password = "rayyan142";

WebSocketsClient webSocket;



/** Audio buffers, pointers and selectors */
typedef struct {
    int16_t *buffer;
    uint8_t buf_ready;
    uint32_t buf_count;
    uint32_t n_samples;
} inference_t;

static inference_t inference;
static const uint32_t sample_buffer_size = 2048;
static signed short sampleBuffer[sample_buffer_size];
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static bool record_status = true;

/**
 * @brief      Arduino setup function
 */
void setup()
{
    Serial.begin(115200);

   pinMode(LED_BUILT_IN, OUTPUT); // Set the pin as output
    digitalWrite(LED_BUILT_IN, LOW);
      Serial.println("LED initially OFF");
        Serial.print("Connecting to WiFi...");
WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  // Connect to WebSocket server   192.168.46.152
  Serial.println("Connecting to WebSocket server...");
  webSocket.begin("192.168.113.152",5001,"/");
  
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

    // put your setup code here, to run once:

    // comment out the below line to cancel the wait for USB connection (needed for native USB)
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo");
    

    I2S.setAllPins(-1, 42, 41, -1, -1);
    if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS)) {
      Serial.println("Failed to initialize I2S!");
    while (1) ;
  }
    
    // summary of inferencing settings (from model_metadata.h)
    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: ");
    ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf(" ms.\n");
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

    ei_printf("\nStarting continious inference in 2 seconds...\n");
    ei_sleep(2000);

    if (microphone_inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT) == false) {
        ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        return;
    }

    ei_printf("Recording...\n");
}

/**
 * @brief      Arduino main function. Runs the inferencing loop.
 */
 bool led_state = false;
// Add these at the top of your file
unsigned long lastInferenceTime = 0;     // Tracks last inference time
const unsigned long inferenceInterval = 2000;  // Set interval time (in ms)

void loop() {
    webSocket.loop(); // Call WebSocket loop more frequently

    if (millis() - lastInferenceTime >= inferenceInterval) {
        lastInferenceTime = millis();

        bool m = microphone_inference_record();
        if (!m) {
            ei_printf("ERR: Failed to record audio...\n");
            return;
        }

        signal_t signal;
        signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
        signal.get_data = &microphone_audio_signal_get_data;
        ei_impulse_result_t result = { 0 };

        EI_IMPULSE_ERROR r = run_classifier(&signal, &result, debug_nn);
        if (r != EI_IMPULSE_OK) {
            ei_printf("ERR: Failed to run classifier (%d)\n", r);
            return;
        }

        // Print the predictions
        ei_printf("Predictions ");
        ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
        ei_printf(": \n");
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("    %s: ", result.classification[ix].label);
            ei_printf_float(result.classification[ix].value);
            ei_printf("\n");
        }
        
        if (result.classification[0].value > 0.3) {
            led_state = true;
            sendMessage("light", "livingroom", "on"); // "On" command
        } 
        else if (result.classification[1].value > 0.3) {
            led_state = false;
            sendDim("television","50","livingroom"); // "Off" command
        }
         else if (result.classification[2].value > 0.3) {
            // led_state = false;
 sendMessage("ac", "livingroom", "on");        }
          else if (result.classification[3].value > 0.4) {
            // led_state = false;
            sendMessage("light", "livingroom", "on"); // "Off" command
        }
         else if (result.classification[3].value > 0.5) {
            sendMessage("Fan","livingroom","alloff"); // "Off" command
        }
  else if (result.classification[4].value > 0.3) {
            sendMessage("Fan","livingroom","on"); // "Off" command
        }
    #if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("    anomaly score: ");
        ei_printf_float(result.anomaly);
        ei_printf("\n");
    #endif
    }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_DISCONNECTED) {
    Serial.println("Disconnected from WebSocket server");
  } else if (type == WStype_CONNECTED) {
    Serial.println("Connected to WebSocket server");
  }
}
void sendMessage(const char* device, const char* room, const char* status) {
  StaticJsonDocument<200> doc;
  doc["device"] = device;
    doc["status"] = status;
  doc["room"] = room;

  String jsonString;
  serializeJson(doc, jsonString);
  
  webSocket.sendTXT(jsonString);  // Send JSON message over WebSocket

}
void sendcontrol(const char* device, const char* room, const char* temp) {
  StaticJsonDocument<200> doc;
  doc["device"] = device;
    doc["control"] = temp;
  doc["room"] = room;

  String jsonString;
  serializeJson(doc, jsonString);
  
  webSocket.sendTXT(jsonString);  // Send JSON message over WebSocket

}
void sendDim(const char* device, const char* room, const char* temp) {
  StaticJsonDocument<200> doc;
  doc["device"] = device;
    doc["brightness"] = temp;
  doc["room"] = room;

  String jsonString;
  serializeJson(doc, jsonString);
  
  webSocket.sendTXT(jsonString);  // Send JSON message over WebSocket

}

//  if (webSocket.isConnected()) {
//         StaticJsonDocument<200> doc;
//         doc["room"] = "bedroom";
//         doc["temperature"] = temp;

//         String message;
//         serializeJson(doc, message);
//         webSocket.sendTXT(message);
//         Serial.println("Temperature update sent: " + message);
static void audio_inference_callback(uint32_t n_bytes)
{
    for(int i = 0; i < n_bytes>>1; i++) {
        inference.buffer[inference.buf_count++] = sampleBuffer[i];

        if(inference.buf_count >= inference.n_samples) {
          inference.buf_count = 0;
          inference.buf_ready = 1;
        }
    }
}

static void capture_samples(void* arg) {

  const int32_t i2s_bytes_to_read = (uint32_t)arg;
  size_t bytes_read = i2s_bytes_to_read;

  while (record_status) {

    /* read data at once from i2s */
    esp_i2s::i2s_read(esp_i2s::I2S_NUM_0, (void*)sampleBuffer, i2s_bytes_to_read, &bytes_read, 100);

    if (bytes_read <= 0) {
      ei_printf("Error in I2S read : %d", bytes_read);
    }
    else {
        if (bytes_read < i2s_bytes_to_read) {
        ei_printf("Partial I2S read");
        }

        // scale the data (otherwise the sound is too quiet)
        for (int x = 0; x < i2s_bytes_to_read/2; x++) {
            sampleBuffer[x] = (int16_t)(sampleBuffer[x]) * 8;
        }

        if (record_status) {
            audio_inference_callback(i2s_bytes_to_read);
        }
        else {
            break;
        }
    }
  }
  vTaskDelete(NULL);
}

/**
 * @brief      Init inferencing struct and setup/start PDM
 *
 * @param[in]  n_samples  The n samples
 *
 * @return     { description_of_the_return_value }
 */
static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));

    if(inference.buffer == NULL) {
        return false;
    }

    inference.buf_count  = 0;
    inference.n_samples  = n_samples;
    inference.buf_ready  = 0;

//    if (i2s_init(EI_CLASSIFIER_FREQUENCY)) {
//        ei_printf("Failed to start I2S!");
//    }

    ei_sleep(100);

    record_status = true;

    xTaskCreate(capture_samples, "CaptureSamples", 1024 * 32, (void*)sample_buffer_size, 10, NULL);

    return true;
}

/**
 * @brief      Wait on new data
 *
 * @return     True when finished
 */
static bool microphone_inference_record(void)
{
    bool ret = true;

    while (inference.buf_ready == 0) {
        delay(10);
    }

    inference.buf_ready = 0;
    return ret;
}

/**
 * Get raw audio signal data
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffer[offset], out_ptr, length);

    return 0;
}

/**
 * @brief      Stop PDM and release buffers
 */
static void microphone_inference_end(void)
{
    free(sampleBuffer);
    ei_free(inference.buffer);
}

//
//static int i2s_init(uint32_t sampling_rate) {
//  // Start listening for audio: MONO @ 8/16KHz
//  i2s_config_t i2s_config = {
//      .mode = (i2s_mode_t)(I2S_CHANNEL_MONO),
//      .sample_rate = sampling_rate,
//      .bits_per_sample = (i2s_bits_per_sample_t)16,
//      .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
//      .communication_format = I2S_COMM_FORMAT_I2S,
//      .intr_alloc_flags = 0,
//      .dma_buf_count = 8,
//      .dma_buf_len = 512,
//      .use_apll = false,
//      .tx_desc_auto_clear = false,
//      .fixed_mclk = -1,
//  };
//  i2s_pin_config_t pin_config = {
//      .bck_io_num = -1,    // IIS_SCLK 26
//      .ws_io_num = 42,     // IIS_LCLK 32
//      .data_out_num = -1,  // IIS_DSIN -1
//      .data_in_num = 41,   // IIS_DOUT 33
//  };
//  esp_err_t ret = 0;
//
//  ret = i2s_driver_install((i2s_port_t)1, &i2s_config, 0, NULL);
//  if (ret != ESP_OK) {
//    ei_printf("Error in i2s_driver_install");
//  }
//
//  ret = i2s_set_pin((i2s_port_t)1, &pin_config);
//  if (ret != ESP_OK) {
//    ei_printf("Error in i2s_set_pin");
//  }
//
//  ret = i2s_zero_dma_buffer((i2s_port_t)1);
//  if (ret != ESP_OK) {
//    ei_printf("Error in initializing dma buffer with 0");
//  }
//
//  return int(ret);
//}
//
//static int i2s_deinit(void) {
//    i2s_driver_uninstall((i2s_port_t)1); //stop & destroy i2s driver
//    return 0;
//}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif


