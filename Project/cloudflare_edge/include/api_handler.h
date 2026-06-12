#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void        edgeInit();
const char* edgeHandle(const char* method, const char* path,
                       const char* query,  const char* body,
                       const char* clientIP);
void        edgeFreeString(const char* ptr);
int         edgeGetRequestCount();
int         edgeGetRateRemaining(const char* clientIP);

#ifdef __cplusplus
}
#endif
