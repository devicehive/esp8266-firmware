/**
 * @file
 * @brief Base64 encode/decode.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DATAENCODEBASE64_H_
#define _DATAENCODEBASE64_H_

#include <c_types.h>


/**
 * @brief Encode binary data to text using `Base64` encoding.
 *
 * Function checks if length of output buffer is enough to store `Base64`
 * output text. If output buffer length is too small nothing is written.
 *
 * @param[in] data Pointer to binary data.
 * @param[in] data_len Binary data length in bytes.
 * @param[out] text Pointer to output buffer.
 * @param[in] text_len Output buffer length in bytes.
 * @return Number of bytes written to output buffer.
 */
int esp_base64_encode(const void *data,
                      size_t data_len,
                      char *text,
                      size_t text_len);


/**
 * @brief Estimate how many bytes will take `Base64` encoded data.
 * @param[in] data_len Binary data length in bytes.
 * @return `Base64` encoded text length in bytes.
 */
size_t esp_base64_encode_length(size_t data_len);


/**
 * @brief Decode text to binary data using `Base64` encoding.
 *
 * Function checks if length of output buffer is enough to store `Base64`
 * decoded data. If output buffer length is too small nothing is written.
 * @param[in] text Pointer to `Base64` encoded text.
 * @param[in] text_len Encoded text length in bytes. Should be multiple of 4.
 * @param[out] data Pointer to output binary data buffer.
 * @param[in] data_len Output buffer length in bytes.
 * @return Number of bytes written to output buffer.
 */
int esp_base64_decode(const char *text,
                      size_t text_len,
                      void *data,
                      size_t data_len);


/**
 * @brief Estimate how many bytes will take base64 decoded data.
 *
 * Encoded data is used to check padding characters at the end.
 *
 * @param[in] text Pointer to base64 encoded text.
 * @param[in] text_len Encoded text length in bytes.
 * @return Decoded binary data length in bytes.
 */
size_t esp_base64_decode_length(const char *text, size_t text_len);

#endif /* _DATAENCODEBASE64_H_ */
