#ifndef __DEBUG_TRANSPORT_H__
#define __DEBUG_TRANSPORT_H__

#ifdef __cplusplus
extern "C" {
#endif

void debug_transport_init(void);
void debug_transport_write_string(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_TRANSPORT_H__ */
