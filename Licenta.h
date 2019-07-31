
typedef NTSTATUS(NTAPI *_NtQueryInformationProcess)(
	HANDLE ProcessHandle,
	DWORD ProcessInformationClass,
	PVOID ProcessInformation,
	DWORD ProcessInformationLength,
	PDWORD ReturnLength
);

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PROCESS_BASIC_INFORMATION
{
	LONG ExitStatus;
	PVOID PebBaseAddress;
	ULONG_PTR AffinityMask;
	LONG BasePriority;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR ParentProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;
typedef struct PACKET
{
	char * image;
	DWORD pid;
	double pageFault;
	double workSetSizeR;
	double pagePool;
	double nonPagedPool;
	double cpuUsage;
	DWORD hasDll;
	DWORD isWOW64;
	DWORD isAdmin;
	
	double info[8];
	
	DWORD ppid;
	DWORD exitCode;
	DWORD version;
	char * commandLine;
} PACKET;




//----------------------------------Windows Service Function Declarations---------------------------------------------------------//
void  WINAPI ServiceMain(DWORD dwargc, LPTSTR *lpargv);
void WINAPI ServiceControlHandler(DWORD dwControl);
void ServiceReportStatus(DWORD dwCurrentStatus, DWORD dwWin32ExitCode, DWORD dwWaitHint);
BOOL ServiceInit(DWORD dwArgc, LPTSTR *lpArgv);
BOOL ServiceInstall(void);
BOOL ServiceDelete(void);
BOOL ServiceStart(void);
BOOL ServiceStop(void);
BOOL ServiceEnum(DWORD processID);
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
int advencedInfoAboutInstalledDevices();
int packetsWithoutCallback();
char* ip6tos(struct sockaddr *sockaddr, char *address, int addrlen);
void WINAPI  PrintStruct(PACKET * p, int nr);
int * Kmeans(int K, double ** points, int nrPoints, double ** centroids, int dim, int j, int *nrAnom);

