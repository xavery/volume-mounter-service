#include <windows.h>

#include <aclapi.h>
#include <stdio.h>
#include <tchar.h>

#define BUFSIZE 512

#ifdef UNICODE
#error "nope"
#endif

static void allowAllAccess(SECURITY_ATTRIBUTES *sa) {
  /* setting the security descriptor's DACL to a null value allows all access
   * to the object. TODO : do this properly - I don't really know what kind of
   * security implications this might have. */
  SECURITY_DESCRIPTOR *sd =
      HeapAlloc(GetProcessHeap(), 0, SECURITY_DESCRIPTOR_MIN_LENGTH);
  InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
  SetSecurityDescriptorDacl(sd, TRUE, NULL, FALSE);

  sa->nLength = sizeof(SECURITY_ATTRIBUTES);
  sa->lpSecurityDescriptor = sd;
  sa->bInheritHandle = FALSE;
}

/* 8 bytes + dash + 3 groups of 4 bytes + 3 dashes + 12 bytes */
#define ASCII_GUID_SIZE (8 + 1 + (3 * 4) + 3 + 12)

static void processMountRequest(const char *asciiGuid, char driveLetter) {
  char volumePath[64] = {0};
  char drivePath[8] = {0};
  _snprintf_s(volumePath, sizeof(volumePath), _TRUNCATE,
              "\\\\?\\Volume{%.*s}\\", ASCII_GUID_SIZE, asciiGuid);
  _snprintf_s(drivePath, sizeof(drivePath), _TRUNCATE, "%c:\\", driveLetter);
  SetVolumeMountPointA(drivePath, volumePath);
}

static void processUnmountRequest(char driveLetter) {
  char drivePath[16] = {0};
  _snprintf_s(drivePath, _TRUNCATE, sizeof(drivePath), "%c:\\", driveLetter);
  DeleteVolumeMountPointA(drivePath);
}

#define REQUEST_TYPE_MOUNT 0
#define REQUEST_TYPE_UNMOUNT 1

static void ProcessClient(HANDLE hPipe) {
  char header[2];
  DWORD bytesRead;
  BOOL success = ReadFile(hPipe, &header, sizeof(header), &bytesRead, 0);

  if (!success || bytesRead != sizeof(header))
    goto beach;

  switch (header[0]) {
  case REQUEST_TYPE_MOUNT: {
    char guid[ASCII_GUID_SIZE];
    success = ReadFile(hPipe, &guid, sizeof(guid), &bytesRead, 0);
    if (!success || bytesRead != sizeof(guid))
      goto beach;
    processMountRequest(guid, header[1]);
  } break;
  case REQUEST_TYPE_UNMOUNT:
    processUnmountRequest(header[1]);
    break;
  }

beach:
  FlushFileBuffers(hPipe);
  DisconnectNamedPipe(hPipe);
  CloseHandle(hPipe);
}

static HANDLE createServerPipeHandle(void) {
  static const char *pipeName = "\\\\.\\pipe\\VolumeMounterService";
  SECURITY_ATTRIBUTES sa;
  allowAllAccess(&sa);
  return CreateNamedPipeA(pipeName, PIPE_ACCESS_DUPLEX,
                          PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT |
                              PIPE_REJECT_REMOTE_CLIENTS,
                          1, BUFSIZE, BUFSIZE, 0, &sa);
}

int main(void) {
  while (1) {
    BOOL connected;
    HANDLE hPipe = createServerPipeHandle();
    if (hPipe == INVALID_HANDLE_VALUE) {
      return 1;
    }

    connected = ConnectNamedPipe(hPipe, NULL)
                    ? TRUE
                    : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (connected)
      ProcessClient(hPipe);
    else
      CloseHandle(hPipe);
  }

  return 0;
}
