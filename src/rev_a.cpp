#include <Arduino.h>

// build the other project, and then convert its firmware.bin to a header
#define FIRMWARE_REV_B_ZIP_IMPLEMENTATION
#include <firmware_rev_b_zip.h>
#include <htcw_zip.hpp>
#include "esp_ota_ops.h"
using namespace zip;
using namespace io;

esp_ota_handle_t handle = 0;
uint8_t write_buffer[8192];
size_t write_size = sizeof(write_buffer);
uint8_t* write_ptr = write_buffer;
void setup() {
  Serial.begin(115200);
  Serial.println("Hello from revision A");
  Serial.print("Unpacking and updating firmware (");
  Serial.print(sizeof(firmware_rev_b_zip));
  Serial.println(" bytes)");
  archive arch;
  io::const_buffer_stream in(firmware_rev_b_zip,sizeof(firmware_rev_b_zip));
  in.seek(0);
  zip_result r = archive::open(&in,&arch);
  if(r!=zip_result::success) {
      Serial.print("Unpacking error: ");
      Serial.println((int)r);
      while(1);
  }
  Serial.print("Archive entries: ");
  Serial.println(arch.entries_size());
  archive_entry entry;
  arch.entry(0,&entry);
  if(!entry.initialized()) {
    Serial.println("Archive entry not initialized");
    while(1);
  }
  esp_ota_begin(esp_ota_get_next_update_partition(NULL), OTA_SIZE_UNKNOWN, &handle);
  r=entry.extract([](const uint8_t* buffer,size_t size, void* state){
    if(size>write_size) {
      size_t sz = sizeof(write_buffer)-write_size;
      if(ESP_OK!=esp_ota_write(handle,write_buffer,sz)) {
        Serial.println("OTA write error");
        return (size_t)0;
      } else {
        Serial.print("OTA wrote ");
        Serial.print(sz);
        Serial.println(" bytes");
      }
      write_size = sizeof(write_buffer);
      write_ptr = write_buffer;
    }
    memcpy(write_ptr,buffer,size);
    write_ptr+=size;
    write_size-=size;
    return size;
  });
  if(zip_result::success==r) {
    if(write_size<sizeof(write_buffer)) {
      size_t sz = sizeof(write_buffer)-write_size;
      if(ESP_OK!=esp_ota_write(handle,write_buffer,sz)) {
        Serial.println("OTA write error");
        while(1);
      } else {
        Serial.print("OTA wrote ");
        Serial.print(sz);
        Serial.println(" bytes");
      }
    }
    if (ESP_OK == esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL))) {
      Serial.println("Updated. Restarting");
      esp_restart();
    } else {
      Serial.println("OTA unable to set boot partition");
      Serial.println("Update error");
    }
  } else {
    if(r!=zip_result::success) {
      Serial.print("Unpacking error: ");
      Serial.println((int)r);
    }
    Serial.println("Update error");
  }
}

void loop() {

}
