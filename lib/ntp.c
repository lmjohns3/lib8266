#include <sys/time.h>

#include "lib8266/ntp.h"
#include "lib8266/wifi.h"

#include "lwip/sockets.h"

#define SEC_1900_TO_1970 2208988800

static const char *TAG = "⚓ ntp";

#define NTP_REQUEST ("\x1b\x00\x00\x00\x00\x00\x00\x00"\
                     "\x00\x00\x00\x00\x00\x00\x00\x00"\
                     "\x00\x00\x00\x00\x00\x00\x00\x00"\
                     "\x00\x00\x00\x00\x00\x00\x00\x00"\
                     "\x00\x00\x00\x00\x00\x00\x00\x00"\
                     "\x00\x00\x00\x00\x00\x00\x00\x00")

static inline uint32_t ntoh(uint8_t *b) {
  uint32_t result = 0;
  result |= *(b+0);
  result <<= 8;
  result |= *(b+1);
  result <<= 8;
  result |= *(b+2);
  result <<= 8;
  result |= *(b+3);
  return result;
}

esp_err_t ahoy_ntp_sync() {
  const int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0) {
    ESP_LOGE(TAG, "error creating UDP socket: %d", errno);
    return ESP_ERR_TIMEOUT;
  }

  struct sockaddr_in dest, sndr;
  dest.sin_addr.s_addr = inet_addr("5.161.44.72");
  dest.sin_family = AF_INET;
  dest.sin_port = htons(123);

  socklen_t socklen = sizeof(sndr);

  ESP_LOGI(TAG, "sending NTP request ...");
  const int64_t mark = esp_timer_get_time();
  if (sendto(sock, NTP_REQUEST, 48, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
    ESP_LOGE(TAG, "sendto failed: errno %d", errno);
  } else {
    uint8_t rx[49];
    const int len = recvfrom(sock, rx, 48, 0, (struct sockaddr *)&sndr, &socklen);
    if (len != 48) {
      ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
    } else {
      const int64_t elapsed_usec = esp_timer_get_time() - mark;
      ESP_LOGD(TAG, "received NTP response after %u usec:", (uint32_t)elapsed_usec);

      const uint8_t li = rx[0] & 0b11000000;
      const uint8_t vn = rx[0] & 0b00111000;
      const uint8_t mode = rx[0] & 0b00000111;
      const uint8_t stratum = rx[1];
      const uint8_t poll_interval = rx[2];
      const uint8_t precision = rx[3];
      ESP_LOGD(TAG, "+ li %d vn %d mode %d stratum %d poll_interval %d precision %d",
               li, vn, mode, stratum, poll_interval, precision);

      uint32_t root_delay = ntoh(rx + 4);
      ESP_LOGD(TAG, "+ root delay %u", root_delay);
      uint32_t root_dispersion = ntoh(rx + 8);
      ESP_LOGD(TAG, "+ root dispersion %u", root_dispersion);
      uint32_t reference_id = ntoh(rx + 12);
      ESP_LOGD(TAG, "+ reference id %u", reference_id);
      int64_t ref_sec = ntoh(rx + 16), ref_frac = ntoh(rx + 20);
      ESP_LOGD(TAG, "+ reference time %u.%u", (uint32_t)ref_sec, (uint32_t)ref_frac);
      int64_t orig_sec = ntoh(rx + 24), orig_frac = ntoh(rx + 28);
      ESP_LOGD(TAG, "+ original time %u.%u", (uint32_t)orig_sec, (uint32_t)orig_frac);
      int64_t rcv_sec = ntoh(rx + 32), rcv_frac = ntoh(rx + 36);
      ESP_LOGD(TAG, "+ receive time %u.%u", (uint32_t)rcv_sec, (uint32_t)rcv_frac);
      int64_t xmit_sec = ntoh(rx + 40), xmit_frac = ntoh(rx + 44);
      ESP_LOGD(TAG, "+ transmit time %u.%u", (uint32_t)xmit_sec, (uint32_t)xmit_frac);

      struct timeval tv = { .tv_sec = (time_t)(xmit_sec - SEC_1900_TO_1970), .tv_usec = 0 };
      settimeofday(&tv, 0);
    }
  }

  shutdown(sock, 0);
  close(sock);

  return ESP_OK;
}
