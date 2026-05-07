#ifndef __BLM_WAVEFORM_TEST_H__
#define __BLM_WAVEFORM_TEST_H__

/**
 * @brief Initialize waveform test channels
 */
void blm_waveform_test_init(void);

/**
 * @brief Push sine/cosine samples to mwaveform
 *        Should be called in a 1ms interrupt
 */
void blm_waveform_test_step(void);

#endif /* __BLM_WAVEFORM_TEST_H__ */
