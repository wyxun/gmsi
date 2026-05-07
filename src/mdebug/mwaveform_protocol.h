#ifndef __MWAVEFORM_PROTOCOL_H__
#define __MWAVEFORM_PROTOCOL_H__

#include <stdint.h>

#define MWAVEFORM_PROTOCOL_MAX_CHANNELS 16

typedef struct {
    char    achName[8];
    float   fScale;
} mwaveform_ch_desc_t;

typedef struct {
    /**
     * @brief Pack data frame
     * @param pchBuffer Output buffer
     * @param ahwSamples Array of samples
     * @param abMask Mask of active channels
     * @param chCount Total number of registered channels
     * @param chSeq Sequence number
     * @return uint16_t Length of packed frame
     */
    uint16_t (*pack_data)(uint8_t *pchBuffer, const int16_t *ahwSamples, const uint8_t *abMask, uint8_t chCount, uint8_t chSeq);
    
    /**
     * @brief Pack descriptor frame
     * @param pchBuffer Output buffer
     * @param atChannels Array of channel descriptors
     * @param chCount Total number of registered channels
     * @return uint16_t Length of packed frame
     */
    uint16_t (*pack_desc)(uint8_t *pchBuffer, const mwaveform_ch_desc_t *atChannels, uint8_t chCount);

} mwaveform_protocol_t;

extern const mwaveform_protocol_t default_waveform_protocol;

#endif /* __MWAVEFORM_PROTOCOL_H__ */
