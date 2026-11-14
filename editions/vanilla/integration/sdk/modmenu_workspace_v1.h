#ifndef MODMENU_WORKSPACE_V1_H
#define MODMENU_WORKSPACE_V1_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Resolve these names from the already-loaded ModMenuBedrock.dll.
 * UTF-8 bytes are copied within the call; no callback or pointer is retained.
 * Call from a plugin worker, not DllMain, input or rendering callbacks.
 * 0 OK; 1 short buffer; -1 invalid; -2 not Ready; -3 identity/refused.
 * Poll required includes NUL; 0 means no command. Short Poll never dequeues.
 */
typedef int32_t (*MMB_WorkspaceRegisterV1Fn)(const void* owner_address,const char* declaration,uint32_t bytes,uint64_t* session);
typedef int32_t (*MMB_WorkspacePublishV1Fn)(uint64_t session,const char* state,uint32_t bytes);
typedef int32_t (*MMB_WorkspacePollV1Fn)(uint64_t session,char* output,uint32_t capacity,uint32_t* required);
typedef int32_t (*MMB_WorkspaceUnregisterV1Fn)(uint64_t session);
#ifdef __cplusplus
}
#endif
#endif
