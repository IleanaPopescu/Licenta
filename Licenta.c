// Licenta.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "pch.h"

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <winsvc.h>
#include <psapi.h>
#include <TlHelp32.h>
#include "Licenta.h"


#define SLEEP_TIME 5000
#define SERVICE_NAME   TEXT("Ileana Service")    // Service name

//----------------------------------Global Variable Decalrations----------------------------------------------------------------------//
SERVICE_STATUS ServiceStatus = { 0 };   //  Service Status Structure
SERVICE_STATUS_HANDLE hServiceStatus = NULL;  //  Service Status Handle for Register the Service 
HANDLE hServiceEvent = NULL;  // Event Handle for Service
//system total times
FILETIME prevSysKernelTime;
FILETIME prevSysUserTime;

//process times
FILETIME prevProcKernelTime;
FILETIME prevProcUserTime;
double CpuUsage;
FILE *fp;
FILE *fpak;
double * retCPU;
int w;
char ** valvec;

void print_anomali(int *point, PACKET **pachet, int proc, int nrAnom)
{
	for (int i = 0; i < nrAnom; i++) {
		printf("ANOMALIE LA SCANAREA %d \n",proc);
		printf("Image:: %s\n", pachet[proc][point[i]].image);
		printf("PID:: %d\n", pachet[proc][point[i]].pid);

		for (int k = 0; k < 8; k++) {
			printf("%s:: %f\n", valvec[k],pachet[proc][point[i]].info[k]);

		}

		printf("ppid:: %d\n", pachet[proc][point[i]].ppid);
		printf("exitCode:: %d\n", pachet[proc][point[i]].exitCode);
		printf("version:: %d\n", pachet[proc][point[i]].version);
		printf("commandLine:: %s\n", pachet[proc][point[i]].commandLine);
	}
}
void minmax(double * vect, double * min, double * max, int nr);

//----------------------------------Main Function--------------------------------------------------------------------------------------//
void main(int argc, CHAR **argv)
{
	printf("IN MAIN FUNCTION\n");

	BOOL bServiceCtrlDispacher = FALSE;

	if (lstrcmpA(argv[1], "install") == 0) {
		ServiceInstall();
		printf("Install Success\n");
	}
    else if (lstrcmpA(argv[1], "start") == 0) {
        ServiceStart();
        printf("ServiceStart Success\n");

        int contor = 0;
        char  nameFile[20];
        DWORD nrProc[10] = {0};
		int vec_min[10] = { 0 };
		int vec_max[10] = { 0 };
		valvec = (char **)malloc(sizeof(char *) * 8);

		valvec[0] = _strdup("pageFault");
		valvec[1] = _strdup("workSetSizeR");
		valvec[2] = _strdup("pagePool");
		valvec[3] = _strdup("nonPagedPool");
		valvec[4] = _strdup("cpuUsage");
		valvec[5] = _strdup("hasDll");
		valvec[6] = _strdup("isWOW64");
		valvec[7] = _strdup("isAdmin");

        while (1) 
        {
            contor++;
           
            sprintf_s(nameFile, 20, "%d.txt", contor);
			errno_t err = fopen_s(&fp, "process.txt", "w+");
            errno_t errp = fopen_s(&fpak, nameFile, "w+");
			
            // Get the list of process identifiers.
			DWORD aProcesses[1024], cbNeeded, cProcesses;
			unsigned int i;

			if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
			{
				return;
			}

			// Calculate how many process identifiers were returned.
			cProcesses = cbNeeded / sizeof(DWORD);

            int c = 0;

			retCPU = (double *)malloc(sizeof(double) *cProcesses);
			w = 0;
			// Print the name and process identifier for each process.
			for (i = 0; i < cProcesses; i++)
			{
				if (aProcesses[i] != 0)
				{             
                    ZeroMemory(&prevSysKernelTime, sizeof(FILETIME));
                    ZeroMemory(&prevSysUserTime, sizeof(FILETIME));
                    ZeroMemory(&prevProcKernelTime, sizeof(FILETIME));
                    ZeroMemory(&prevProcUserTime, sizeof(FILETIME));
                    if (ServiceEnum(aProcesses[i])) {
                        c++;
                    }
				}
			}
			double min, max;
			minmax(retCPU, &min, &max, c);

			vec_max[contor - 1] = max;
			vec_min[contor - 1] = min;

            nrProc[contor - 1] = c;
			err = fclose(fp);
			int numclosed = _fcloseall();
			Sleep(3000);
            if (contor == 10)
                break;
		}
        PACKET * packet[10];

        for (int j = contor - 1; j >= 0; j--) {
            sprintf_s(nameFile, 20, "%d.txt", j + 1);
            errno_t errp = fopen_s(&fpak, nameFile, "r+");
          
			printf("CONTORUL DE CITITIRE:: %d\n", j);
            if (errp) {
                printf("EROARE LA %s\n", nameFile);
            }
            packet[j] = (PACKET *)malloc(sizeof(PACKET)* nrProc[j]);

            char *str = (char *)malloc(sizeof(char) * 500);
            for (int i = 0; i < nrProc[j]; i++) {

                fgets(str, 500, fpak);
                packet[j][i].image = _strdup(str);

                fgets(str, 500, fpak);
                packet[j][i].pid = atoi(str);

				for (int k = 0; k < 8; k++) {
					if (k == 0) {
						fgets(str, 500, fpak);
						packet[j][i].info[k] = (atof(str) - vec_min[j]) /( vec_max[j] - vec_min[j]);
						//packet[j][i].info[k] = atof(str);
					}
					else {
						fgets(str, 500, fpak);
						packet[j][i].info[k] = atof(str);
					}
				}
             /*   fgets(str, 500, fpak);
                packet[j][i].pageFault =  atof(str);

                fgets(str, 500, fpak);
                packet[j][i].workSetSizeR = atof(str);

                fgets(str, 500, fpak);
                packet[j][i].pagePool = atof(str);

                fgets(str, 500, fpak);
                packet[j][i].nonPagedPool = atof(str);

                fgets(str, 500, fpak);
                packet[j][i].cpuUsage = atof(str);

                fgets(str, 500, fpak);
                packet[j][i].hasDll = atoi(str);

                fgets(str, 500, fpak);
                packet[j][i].isWOW64 = atoi(str);

                fgets(str, 500, fpak);
                packet[j][i].isAdmin = atoi(str);*/

                fgets(str, 500, fpak);
                packet[j][i].ppid = atoi(str);

                fgets(str, 500, fpak);
                packet[j][i].exitCode = atoi(str);

                fgets(str, 500, fpak);
                packet[j][i].version = atoi(str);

                fgets(str, 500, fpak);
                packet[j][i].commandLine = _strdup(str);

                fgets(str, 500, fpak);
            }

          //  PrintStruct(packet[j], nrProc[j]);
            errp = fclose(fpak);
            printf("cAm citit din :: %s\n", nameFile);

			double ** points = (double **)malloc(sizeof(double *) * nrProc[j]);

			for (int k = 0; k < nrProc[j]; k++) {
				points[k] = (double *)malloc(sizeof(double) * 8);
			}

			double ** centroids = (double **)malloc(sizeof(double *) * 2);

			for (int k = 0; k < 2; k++) {
				centroids[k] = (double *)malloc(sizeof(double) * 8);
			}


			for (int k = 0; k < nrProc[j]; k++) {
				for (int u = 0; u < 8; u++) {
					points[k][u] = packet[j][k].info[u];
				}
			}
			for (int u = 0; u < 8; u++) {
				centroids[0][u] = packet[j][3].info[u];
				centroids[1][u] = packet[j][7].info[u];
			}
			int * rez;
			int nrAnom;
			rez = Kmeans(2, points, nrProc[j], centroids, 8, j, &nrAnom);

			print_anomali(rez, packet, j, nrAnom);		
        }
    }

	else if (lstrcmpA(argv[1], "stop") == 0) {
		ServiceStop();
		printf("ServiceStop Success\n");
	}
	else if (lstrcmpA(argv[1], "delete") == 0) {
		ServiceDelete();
		printf("ServiceDelete Success\n");
	}
	else {
		// STEP 1 --> Fill the Service Table Entry (2D Array)
		SERVICE_TABLE_ENTRY ServiceTable[] =
		{
			{(LPWSTR)SERVICE_NAME,  (LPSERVICE_MAIN_FUNCTION)ServiceMain },
			{ NULL, NULL}
		};

		//STEP 2 --> Start Service Contorl Dispacher
		bServiceCtrlDispacher = StartServiceCtrlDispatcher(ServiceTable);

		if (bServiceCtrlDispacher == FALSE) {
			printf("StartServiceCtrlDispatcher Failed\n");
		}
		else {
			printf("StartServiceCtrlDispatcher Success\n");
		}
	}
	
	printf("IN MAIN FUNCTION END\n");
	system("PAUSE");
	return;
}


void WINAPI  PrintStruct(PACKET * packet, int nr)
{
    for (int i = 0; i < nr; i++) {
      
        printf("Image:: %s\n", packet[i].image);
        printf("PID:: %d\n", packet[i].pid);

		for (int k = 0; k < 8; k++) {
			printf("infooo:: %f\n", packet[i].info[k]);
			
		}


       /* printf("pageFault:: %f\n", packet[i].pageFault);
        printf("workSetSizeR:: %f\n", packet[i].workSetSizeR);
        printf("pagePool:: %f\n", packet[i].pagePool);
        printf("nonPagedPool:: %f\n", packet[i].nonPagedPool);
        printf("cpuUsage:: %f\n", packet[i].cpuUsage);
        printf("hasDll:: %d\n", packet[i].hasDll);
        printf("isWOW64:: %d\n", packet[i].isWOW64);
        printf("isAdmin:: %d\n", packet[i].isAdmin);*/
        printf("ppid:: %d\n", packet[i].ppid);
        printf("exitCode:: %d\n", packet[i].exitCode);
        printf("version:: %d\n", packet[i].version);
        printf("commandLine:: %s\n", packet[i].commandLine);
    }
    printf("numarul:::::: %d\n", nr);
}
void WINAPI  ServiceMain(DWORD dwArgc, LPTSTR *lpArgv)
{
	printf("ServiceMain START\n");

	BOOL bServiceStatus = FALSE;

	// STEP 1 --> Registering Service Control Handler Function to SCM
	hServiceStatus = RegisterServiceCtrlHandler(
								SERVICE_NAME, 
								ServiceControlHandler
							);

	if (hServiceStatus == NULL)
	{
		printf("RegisterServiceCtrlHandler Failed\n");
	}
	else {
		printf("RegisterServiceCtrlHandler Success\n");
	}
	// STEP 2- > SERVICE_STATUS initial SetUp here
	
	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwServiceSpecificExitCode = 0;

	// STEP 3 --> Call Service Report Status for Notifying Initial Setup
	ServiceReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);


	// STEP 4 --> Check the Service Status
	bServiceStatus = SetServiceStatus(hServiceStatus, &ServiceStatus);

	if (bServiceStatus == FALSE) {
		printf("Service Status initial Setup Failed\n");
	}
	else {
		printf("Service Status initial Setup Success\n");
	}

	// STEP 5 --> Call ServiceInit function
	ServiceInit(dwArgc, lpArgv);

	printf("ServiceMain END\n");
}

void ServiceReportStatus(DWORD dwCurrentStatus, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	printf("ServiceReportStatus START\n");
	static DWORD dwCheckPoint = 1;
	BOOL bSetServiceStatus = FALSE;

	// STEP 1 --> Filling the SERVICE_STATUS Structure
	ServiceStatus.dwCurrentState = dwCurrentStatus;
	ServiceStatus.dwServiceSpecificExitCode = dwWin32ExitCode;
	ServiceStatus.dwWaitHint = dwWaitHint;

	// STEP 2 --> Check the Current State of Service
	if (dwCurrentStatus == SERVICE_START_PENDING) {       // Service is about to start
		ServiceStatus.dwControlsAccepted = 0;
	}
	else {
		ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	// STEP 3 --> Progress for Service Operation
	if (dwCurrentStatus == SERVICE_RUNNING || dwCurrentStatus == SERVICE_STOPPED) {
		ServiceStatus.dwCheckPoint = 0;
	}
	else {
		ServiceStatus.dwCheckPoint = dwCheckPoint++;
	}

	// STEP 4 --> Notify the current status of SCM
	bSetServiceStatus = SetServiceStatus(hServiceStatus, &ServiceStatus);
	if (bSetServiceStatus == FALSE) {
		printf("SetServiceStatus Failed\n");
	}
	else {
		printf("SetServiceStatus Success\n");
	}

	printf("ServiceReportStatus END\n");
}

BOOL ServiceInit(DWORD dwArgc, LPTSTR *lpArgv)
{
	printf("ServiceInit START\n");

	//STEP 1 --> Create Event
	hServiceEvent = CreateEvent(NULL, // Security Attributes
								TRUE, //Manual Reset Event
								FALSE, //Non Signaled
								NULL // Name of event
								);

	if (hServiceEvent == NULL) {
		// Call ServicePeportStatus function to notify SCM for Current Status of Service
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
	else {
		// Call ServicePeportStatus function to notify SCM for Current Status of Service
		ServiceReportStatus(SERVICE_RUNNING, NO_ERROR, 0);
	}

	// STEP 2 --> Check whether to stop the service
	while (1) {

		// Wait event to be Signaled
		WaitForSingleObject(hServiceEvent, INFINITE);

		// Send Report status to SCM
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
	printf("ServiceInit END\n");
    return TRUE;
}

void WINAPI  ServiceControlHandler(DWORD dwControl)
{
	printf("ServiceControlHandler START\n");

	switch (dwControl)
	{
		case SERVICE_CONTROL_STOP:
			ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
			printf("Service STOP\n");
			break;

		default:
			break;
	}
	printf("ServiceControlHandler END\n");
}

BOOL ServiceInstall(void)
{
	printf("SERVICEINSTALL START\n");

	SC_HANDLE hOpenSCManager = NULL;
	SC_HANDLE hCreateService = NULL;
	DWORD nGetModuleFIleName = 0;
	TCHAR sizePath[250];
	
	// STEP 1 --> GetModuleFileName Get the Exectable file from SCM
	nGetModuleFIleName = GetModuleFileName(NULL, sizePath, 250);


	if (nGetModuleFIleName == 0) {
		printf("Service Installation Failed\n");
        return FALSE;
	}
	else {
		printf("Succesfully install the File\n");
	}

	//STEP 2 --> Open the Service Control Manager
	hOpenSCManager = OpenSCManager(NULL, //Local Machine
									NULL, // By default database i.e. SERVICE_ACTIVE_DATABASE
									SC_MANAGER_ALL_ACCESS    //All access right
								);

	if (hOpenSCManager == NULL) {
		printf("OpenSCManager Failed\n");
        return FALSE;
	}
	else {
		printf("OpenSCManager Success\n");
	}

	//STEP 3 --> Create the service
	hCreateService = CreateService(hOpenSCManager,
									SERVICE_NAME,
									SERVICE_NAME,
									SERVICE_ALL_ACCESS,
									SERVICE_WIN32_OWN_PROCESS,
									SERVICE_DEMAND_START,
									SERVICE_ERROR_NORMAL,
									sizePath,
									NULL,
									NULL,
									NULL,
									NULL,
									NULL
								);

	if (hCreateService == NULL) {
		printf("CreateService Failed\n");
		CloseServiceHandle(hOpenSCManager);
        return FALSE;
	}
	else {
		printf("CreateService Success\n");
	}

	CloseServiceHandle(hCreateService);
	CloseServiceHandle(hOpenSCManager);

    return TRUE;
}

BOOL ServiceDelete(void)
{
	printf("SERVICEDELETE START");

	SC_HANDLE hOpenSCManager = NULL;
	SC_HANDLE hOpenService = NULL;

	BOOL bDeleteService = NULL;

	//STEP 1 --> Open the Service Control Manager
	hOpenSCManager = OpenSCManager(
		NULL, //Local Machine
		NULL, // By default database i.e. SERVICE_ACTIVE_DATABASE
		SC_MANAGER_ALL_ACCESS   //All access right
	);

	if (hOpenSCManager == NULL) {
		printf("OpenSCManager Failed\n");
        return FALSE;
	}
	else {
		printf("OpenSCManager Success\n");
	}
	
	//STEP 2 --> Open the Service
	hOpenService = OpenService(hOpenSCManager,
								SERVICE_NAME,
								SERVICE_ALL_ACCESS
								);

	if (hOpenService == NULL) {
		printf("OpenService Failed\n");
        return FALSE;
	}
	else {
		printf("OpenService Success\n");
	}

	// STEP 3 --> Delete Service
	bDeleteService = DeleteService(hOpenService);

	if (bDeleteService == NULL) {
		printf("DeleteSetvice Failed\n");
        return FALSE;
	}
	else {
		printf("DeleteService Success\n");
	}

	CloseServiceHandle(hOpenService);
	CloseServiceHandle(hOpenSCManager);
    return TRUE;
}

PVOID GetPebAddress(HANDLE ProcessHandle)
{
	_NtQueryInformationProcess NtQueryInformationProcess =
		(_NtQueryInformationProcess)GetProcAddress(
			GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
	PROCESS_BASIC_INFORMATION pbi;

	NtQueryInformationProcess(ProcessHandle, 0, &pbi, sizeof(pbi), NULL);

	return pbi.PebBaseAddress;
}
DWORD SubtractTimes(const FILETIME& ftA, const FILETIME& ftB)
{
	ULARGE_INTEGER a, b;

	a.LowPart = ftA.dwLowDateTime;
	a.HighPart = ftA.dwHighDateTime;
	
	b.LowPart = ftB.dwLowDateTime;
	b.HighPart = ftB.dwHighDateTime;
	
	return a.QuadPart - b.QuadPart;
}

BOOL SearchTokenGroupsForSID(HANDLE hProcess)
{
    DWORD i, dwSize = 0, dwResult = 0;
    HANDLE hToken;
    PTOKEN_GROUPS pGroupInfo;
    SID_NAME_USE SidType;
    char lpName[MAX_PATH];
    char lpDomain[MAX_PATH];
    PSID pSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;

    // Open a handle to the access token for the calling process.
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
    {
        printf("OpenProcessToken Error %u\n", GetLastError());
        return FALSE;
    }
    // Call GetTokenInformation to get the buffer size.
    if (!GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize))
    {
        dwResult = GetLastError();
        if (dwResult != ERROR_INSUFFICIENT_BUFFER) {
            printf("GetTokenInformation Error %u\n", dwResult);
            return FALSE;
        }
    }
    // Allocate the buffer.

    pGroupInfo = (PTOKEN_GROUPS)GlobalAlloc(GPTR, dwSize);

    // Call GetTokenInformation again to get the group information.
    if (!GetTokenInformation(hToken, TokenGroups, pGroupInfo, dwSize, &dwSize))
    {
        printf("GetTokenInformation Error %u\n", GetLastError());
        return FALSE;
    }
    // Create a SID for the BUILTIN\Administrators group.
    if (!AllocateAndInitializeSid(&SIDAuth, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pSID))
    {
        printf("AllocateAndInitializeSid Error %u\n", GetLastError());
        return FALSE;
    }
    // Loop through the group SIDs looking for the administrator SID.
    int gasit = 0;
    for (i = 0; i < pGroupInfo->GroupCount; i++)
    {
        if (EqualSid(pSID, pGroupInfo->Groups[i].Sid))
        {
            gasit = 1;
           
            // Lookup the account name and print it.
            dwSize = MAX_PATH;
            if (!LookupAccountSid(NULL, pGroupInfo->Groups[i].Sid,
                (LPWSTR)lpName, &dwSize, (LPWSTR)lpDomain,
                &dwSize, &SidType))
            {
                dwResult = GetLastError();
                if (dwResult == ERROR_NONE_MAPPED)
                    strcpy_s(lpName, dwSize, "NONE_MAPPED");
                else
                {
                    printf("LookupAccountSid Error %u\n", GetLastError());
                    return FALSE;
                }
            }
            fprintf(fp, "Current user is a member of the %ls\\%ls group\n",lpDomain, lpName);
            fprintf(fpak, "0\n"); // e in admin

            // Find out whether the SID is enabled in the token.
            if (pGroupInfo->Groups[i].Attributes & SE_GROUP_ENABLED)
                fprintf(fp,"The group SID is enabled.\n");
            else if (pGroupInfo->Groups[i].Attributes &
                SE_GROUP_USE_FOR_DENY_ONLY)
                fprintf(fp,"The group SID is a deny-only SID.\n");
            else
                fprintf(fp,"The group SID is not enabled.\n");
        }
    }
    if(gasit == 0)
        fprintf(fpak, "1\n"); // nu e in admin
    if (pSID)
        FreeSid(pSID);
    if (pGroupInfo)
        GlobalFree(pGroupInfo);
    return TRUE;
}

void minmax(double * vect, double * min, double * max, int nr)
{
	(* max) = vect[0];
	(* min) = vect[0];
	for (int i = 0; i < nr; i++) {
		if (vect[i] > (* max)) {
			(* max) = vect[i];
		}
		else if (vect[i] < (* min)) {
			(* min) = vect[i];
		}
	}
}

BOOL ServiceEnum(DWORD processID)
{
	//printf("ServiceEnum START\n");

	TCHAR lpImageFileName[MAX_PATH];
	PVOID pebAddress;
	PVOID rtlUserProcParamsAddress;
	UNICODE_STRING commandLine;
	WCHAR *commandLineContents;

    BOOL getProcCount;
    IO_COUNTERS * lpIoCounters = (IO_COUNTERS *)malloc(sizeof(IO_COUNTERS));

	FILETIME procCreateTime;
	FILETIME procExitTime;
	FILETIME procKernelTime;
	FILETIME procUserTime;
	FILETIME sysKernelTime;
	FILETIME sysUserTime;
	FILETIME sysIdleTime;
	double nCpuCopy = CpuUsage;

    BOOL isWOW64;
    USHORT *pProcessMachine = (USHORT *)malloc(sizeof(USHORT) * 100);
    USHORT *pNativeMachine = (USHORT *)malloc(sizeof(USHORT) * 100);


    const TCHAR *dlls[] = {TEXT("wow64cpu.dll"), TEXT("wowarmhw.dll"), TEXT("xtajit.dll"), TEXT("advapi32.dll"), TEXT("clbcatq.dll"),
                    TEXT("combase.dll"), TEXT("COMDLG32.dll"), TEXT("coml2.dll"), TEXT("difxapi.dll"), TEXT("gdi32.dll"),
                    TEXT("gdiplus.dll"), TEXT("IMAGEHLP.dll"), TEXT("IMM32.dll"), TEXT("kernel32.dll"), TEXT("MSCTF.dll"),
                    TEXT("MSVCRT.dll"), TEXT("NORMALIZ.dll"), TEXT("NSI.dll"), TEXT("ole32.dll"), TEXT("OLEAUT32.dll"),
                    TEXT("PSAPI.dll"), TEXT("rpcrt4.dll"), TEXT("sechost.dll"), TEXT("Setupapi.dll"), TEXT("SHCORE.dll"),
                    TEXT("SHELL32.dll"), TEXT("SHLWAPI.dll"), TEXT("user32.dll"), TEXT("WLDAP32.dll"), TEXT("wow64.dll"),
                    TEXT("wow64win.dll")};

	TCHAR name[MAX_PATH];
	PROCESS_MEMORY_COUNTERS pmc;
	DWORD exitCode = 0;
	HANDLE hSnapshot;
	DWORD ppid = 0;
	PROCESSENTRY32 pe32;

	// STEP 1 --> Get a handle to the process.
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
									FALSE, processID
								);

	// STEP 2 --> Get the process info.
	if (NULL != hProcess)
	{

        DWORD err = GetProcessImageFileName(hProcess, lpImageFileName, sizeof(lpImageFileName) / sizeof(*lpImageFileName));
        if (err) {
            fprintf(fp, "ImageFileName::  %ls (PID: %u)\n", lpImageFileName, processID);
            fprintf(fpak, "%ls\n%u\n", lpImageFileName, processID);         // Imagine si PID
        }

        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
        {
            fprintf(fp, "The number of page faults: 0x%08X    %d \n", pmc.PageFaultCount, pmc.PageFaultCount);
            fprintf(fpak, "%f\n", (double)pmc.PageFaultCount); // nr de page fault pe secunda
			retCPU[w] = (double)pmc.PageFaultCount;
			w++;

            fprintf(fp, "The peak working set size: 0x%08X   %d  bytes\n", pmc.PeakWorkingSetSize, pmc.PeakWorkingSetSize);

            fprintf(fp, "The current working set size: 0x%08X   %d bytes\n", pmc.WorkingSetSize, pmc.WorkingSetSize);
            double workSetSizeR =  (double)(pmc.WorkingSetSize) / (double)(pmc.PeakWorkingSetSize);
            fprintf(fpak, "%f\n", workSetSizeR);  // The current working set size / The peak working set size

            fprintf(fp, "The peak paged pool usage: 0x%08X   %d bytes\n", pmc.QuotaPeakPagedPoolUsage, pmc.QuotaPeakPagedPoolUsage);

            fprintf(fp, "The current paged pool usage: 0x%08X   %d bytes\n", pmc.QuotaPagedPoolUsage, pmc.QuotaPagedPoolUsage);
            double pagePool = (double)(pmc.QuotaPagedPoolUsage) / (double)(pmc.QuotaPeakPagedPoolUsage);
            fprintf(fpak, "%f\n", pagePool);  //  The current paged pool usage / The peak paged pool usage

            fprintf(fp, "The peak nonpaged pool usage: 0x%08X   %d bytes\n", pmc.QuotaPeakNonPagedPoolUsage, pmc.QuotaPeakNonPagedPoolUsage);

            fprintf(fp, "The current nonpaged pool usage: 0x%08X   %d bytes\n", pmc.QuotaNonPagedPoolUsage, pmc.QuotaNonPagedPoolUsage);
            double nonpagedPool = (double)(pmc.QuotaNonPagedPoolUsage) / (double)(pmc.QuotaPeakNonPagedPoolUsage);
            fprintf(fpak, "%f\n", nonpagedPool);  // The current nonpaged pool usage / The peak nonpaged pool usage

            fprintf(fp, "PagefileUsage: 0x%08X   %d bytes (The Commit Charge value in bytes for this process. Commit Charge is the total amount of memory that the memory manager has committed for a running process.)\n", pmc.PagefileUsage, pmc.PagefileUsage);

            fprintf(fp, "PeakPagefileUsage: 0x%08X   %d bytes (The peak value in bytes of the Commit Charge during the lifetime of this process.)\n", pmc.PeakPagefileUsage, pmc.PeakPagefileUsage);
        }

        // Print time
        if (GetProcessTimes(hProcess, &procCreateTime, &procExitTime, &procKernelTime, &procUserTime) && GetSystemTimes(&sysIdleTime, &sysKernelTime, &sysUserTime))
        {
            ULONGLONG ftSysKernelDiff = SubtractTimes(sysKernelTime, prevSysKernelTime);
            ULONGLONG ftSysUserDiff = SubtractTimes(sysUserTime, prevSysUserTime);
            ULONGLONG ftProcKernelDiff = SubtractTimes(procKernelTime, prevProcKernelTime);
            ULONGLONG ftProcUserDiff = SubtractTimes(procUserTime, prevProcUserTime);
            ULONGLONG nTotalSys = ftSysKernelDiff + ftSysUserDiff;
            ULONGLONG nTotalProc = ftProcKernelDiff + ftProcUserDiff;

            if (nTotalSys > 0) {
                CpuUsage = (double)((double)nTotalProc / (double)nTotalSys);
            }

            fprintf(fp, "CPU Usage::  %f ms\n", CpuUsage);
            fprintf(fpak, "%f\n", CpuUsage);   // Procent CPU Usage

            prevSysKernelTime = sysKernelTime;
            prevSysUserTime = sysUserTime;
            prevProcKernelTime = procKernelTime;
            prevProcUserTime = procUserTime;

            nCpuCopy = CpuUsage;
        }

        int r = 1;
        HMODULE hMods[1024];
        DWORD cbNeeded;
        unsigned int i;

        // Get a list of all the modules in this process.

        if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
        {
            for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
            {
                TCHAR szModName[MAX_PATH];

                // Get the full path to the module's file.

                if (GetModuleBaseName(hProcess, hMods[i], szModName,
                    sizeof(szModName) / sizeof(TCHAR)))
                {
                    // Print the module name and handle value.

                    fprintf(fp, "Module name: %ls and handle value: (0x%08X)\n", szModName, hMods[i]);
                    for (int h = 0; h < 31; h++) {
                        r = _wcsicmp(dlls[h], szModName);
                        if (r == 0) {
                            break;
                        }
                    }
                    if (r == 0)
                        break;
                }
            }
        }

        fprintf(fp, "\t Are sau nu are dll:::  %d\n", r);
        if(r)
            fprintf(fpak, "1\n");    // daca are dll
        else
            fprintf(fpak, "0\n");   //  daca nu are dll

        isWOW64 = IsWow64Process2(hProcess, pProcessMachine, pNativeMachine);
       
        if (isWOW64) {
            if ((*pProcessMachine) == 0) {
                fprintf(fp, "\tpProcessMachine  0x%04x Nu e WOW64\n", (*pProcessMachine));
                fprintf(fpak, "0\n");   // Nu e WOW64
            }
            else {
                fprintf(fp, "\tpProcessMachine  0x%08X E WOW64\n", (*pProcessMachine));
                fprintf(fpak, "1\n");  // E WOW64
            }
            fprintf(fp, "\tpNativeMachine  0x%08X\n", (*pNativeMachine));
        }

        SearchTokenGroupsForSID(hProcess);

        getProcCount = GetProcessIoCounters(hProcess, lpIoCounters);

        if (getProcCount) {
            fprintf(fp, "\tReadOperationCount  %llu \n", lpIoCounters->ReadOperationCount);

            fprintf(fp, "\tWriteOperationCount  %llu \n", lpIoCounters->WriteOperationCount);

            fprintf(fp, "\tOtherOperationCount  %llu \n", lpIoCounters->OtherOperationCount);

            fprintf(fp, "\tReadTransferCount  %llu \n", lpIoCounters->ReadTransferCount);

            fprintf(fp, "\tWriteTransferCount  %llu \n", lpIoCounters->WriteTransferCount);

            fprintf(fp, "\tOtherTransferCount  %llu \n", lpIoCounters->OtherTransferCount);
        }

        hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		__try {
			if (hSnapshot == INVALID_HANDLE_VALUE) __leave;

			ZeroMemory(&pe32, sizeof(pe32));
			pe32.dwSize = sizeof(pe32);
			if (!Process32First(hSnapshot, &pe32)) __leave;

			do {
				if (pe32.th32ProcessID == processID) {
					ppid = pe32.th32ParentProcessID;
					break;
				}
			} while (Process32Next(hSnapshot, &pe32));

		}
		__finally {
			if (hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);
		}

		fprintf(fp, "Parent Process ID: %d\n", ppid);
        fprintf(fpak, "%d\n", ppid);

		//Print the process exitcode 
		if (GetExitCodeProcess(hProcess, &exitCode) == FALSE)
		{
			fprintf(fp, "Failed to get exitCode process.\n");
            fprintf(fpak, "-1\n");
		}
		else {
			fprintf(fp, "The process exitcode  is: %d\n ", exitCode);
            fprintf(fpak, "%d\n", exitCode);
		}

        DWORD dwPver = GetProcessVersion(processID);

        // Print the process version.
        fprintf(fp, "Process version is: %d\n ", dwPver);
        fprintf(fpak, "%d\n", dwPver);

		pebAddress = GetPebAddress(hProcess);

		if (!ReadProcessMemory(hProcess, (PCHAR)pebAddress + 0x10, &rtlUserProcParamsAddress, sizeof(PVOID), NULL)) {
			fprintf(fp, "Could not read the address of ProcessParameters!\n");
		}

		/* read the CommandLine UNICODE_STRING structure */
		if (!ReadProcessMemory(hProcess, (PCHAR)rtlUserProcParamsAddress + 0x40, &commandLine, sizeof(commandLine), NULL))
		{
			fprintf(fp, "Could not read CommandLine!\n");
		}

		/* allocate memory to hold the command line */
		commandLineContents = (WCHAR *)malloc(commandLine.Length);

		if (!ReadProcessMemory(hProcess, commandLine.Buffer,
			commandLineContents, commandLine.Length, NULL))
		{
			fprintf(fp, "Could not read the command line string!\n\n\n");
            fprintf(fpak, " \n\n");
			return TRUE;
		}

		// Print Command Line
		fprintf(fp, "Command Line::  %.*S\n", commandLine.Length / 2, commandLineContents);
        fprintf(fpak, "%.*S\n", commandLine.Length / 2, commandLineContents);
		fprintf(fp, "\n");
        fprintf(fpak, "\n");
        return TRUE;
	}
	
	// Release the handle to the process.
	CloseHandle(hProcess);

	//printf("ServiceEnum STOP\n");
    return FALSE;
}

BOOL ServiceStart(void)
{
	printf("SERVICESTART START\n");

	BOOL bServiceStart = FALSE;
	SERVICE_STATUS_PROCESS sStatusProcess;
	SC_HANDLE hOpenSCManager = NULL;
	SC_HANDLE hOpenService = NULL;
	BOOL bQueryServiceStatus = FALSE;
	DWORD dByteNeeded;

	//STEP 1 --> Open the Service Control Manager
	hOpenSCManager = OpenSCManager(	NULL, //Local Machine
									NULL, // By default database i.e. SERVICE_ACTIVE_DATABASE
									SC_MANAGER_ALL_ACCESS   //All access right
								);

	if (hOpenSCManager == NULL) {
		printf("OpenSCManager Failed\n");
        return FALSE;
	}
	else {
		printf("OpenSCManager Success\n");
	}

	//STEP 2 --> Open the Service
	hOpenService = OpenService(hOpenSCManager,
								SERVICE_NAME,
								SERVICE_ALL_ACCESS
								);

	if (hOpenService == NULL) {
		printf("OpenService Failed\n");
        return FALSE;
	}
	else {
		printf("OpenService Success\n");
	}

	// STEP 3 --> Query about current Service Status
	bQueryServiceStatus = QueryServiceStatusEx(hOpenService,
												SC_STATUS_PROCESS_INFO,
												(LPBYTE)&sStatusProcess,
												sizeof(SERVICE_STATUS_PROCESS),
												&dByteNeeded
											);

	if (bQueryServiceStatus == NULL) {
		printf("QueryServiceStatusEx Failed\n");
        return FALSE;
	}
	else {
		printf("QueryServiceStatusEx Success\n");
	}

	// STEP 4 --> Checked Service is running or stopped
	if (sStatusProcess.dwCurrentState != SERVICE_STOPPED  && sStatusProcess.dwCurrentState != SERVICE_STOP_PENDING) {
		printf("Service is already running\n");
	}
	else {
		printf("Service is already stopped\n");
	}

	// STEP 5 --> If service is stopped then query the service
	while (sStatusProcess.dwCurrentState == SERVICE_STOP_PENDING) {
		bQueryServiceStatus = QueryServiceStatusEx(
										hOpenService,
										SC_STATUS_PROCESS_INFO,
										(LPBYTE)&sStatusProcess,
										sizeof(SERVICE_STATUS_PROCESS),
										&dByteNeeded
									);

		if (bQueryServiceStatus == NULL) {
			printf("QueryServiceStatusEx Failed\n");
		}
		else {
			printf("QueryServiceStatusEx Success\n");
		}
	}

	// STEP 6 --> Start the Service
	bServiceStart = StartService(
							hOpenService,
							NULL,
							NULL
							);

	if (bServiceStart == NULL) {
		printf("StartService Failed\n");
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
        return FALSE;
	}
	else {
		printf("StartService Success\n");
	}

	// STEP 7 --> Query the service agan
	bQueryServiceStatus = QueryServiceStatusEx(
		hOpenService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&sStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dByteNeeded
	);

	if (bQueryServiceStatus == NULL) {
		printf("QueryServiceStatusEx Failed\n");
	}
	else {
		printf("QueryServiceStatusEx Success\n");
	}

	// STEP 8 --> Check Service is running or not
	if (sStatusProcess.dwCurrentState == SERVICE_RUNNING) {
		printf("Service Started Running...\n");
	}
	else {
		printf("Service Started Failed\n");
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
        return FALSE;
	}

	// STEP 9 --> Close the Service Handle for OpenSCManager and OpenService
	CloseServiceHandle(hOpenService);
	CloseServiceHandle(hOpenSCManager);
	printf("SERVICESTART END\n");
    return TRUE;
}

BOOL ServiceStop(void)
{
	printf("Inside Service Stop\n");
	SERVICE_STATUS_PROCESS sStatusProcess;
	SC_HANDLE hOpenSCManager = NULL;
	SC_HANDLE hOpenService = NULL;
	BOOL bQueryServiceStatus = TRUE;
	BOOL bControlService = TRUE;
	DWORD dByteNeeded;

	//STEP 1 --> Open the Service Control Manager
	hOpenSCManager = OpenSCManager(
									NULL, //Local Machine
									NULL, // By default database i.e. SERVICE_ACTIVE_DATABASE
									SC_MANAGER_ALL_ACCESS   //All access right
								);

	if (hOpenSCManager == NULL) {
		printf("OpenSCManager Failed\n");
        return FALSE;
	}
	else {
		printf("OpenSCManager Success\n");
	}

	//STEP 2 --> Open the Service
	hOpenService = OpenService(
							hOpenSCManager,
							SERVICE_NAME,
							SERVICE_ALL_ACCESS
						);

	if (hOpenService == NULL) {
		printf("OpenService Failed\n");
        return FALSE;
	}
	else {
		printf("OpenService Success\n");
	}

	// Step 3 --> QueryServiceStatus
	bQueryServiceStatus = QueryServiceStatusEx(
									hOpenService,
									SC_STATUS_PROCESS_INFO,
									(LPBYTE)&sStatusProcess,
									sizeof(SERVICE_STATUS_PROCESS),
									&dByteNeeded
								);

	if (bQueryServiceStatus == NULL) {
		printf("QueryServiceStatusEx Failed\n");
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
        return FALSE;
	}
	else {
		printf("QueryServiceStatusEx  Success\n");
	}

	// STEP 4 --> send a stop code to the Service Control Manager
	bControlService = ControlService(
								hOpenService,
								SERVICE_CONTROL_STOP,
								(LPSERVICE_STATUS)&sStatusProcess
								);

	if (bControlService == TRUE) {
		printf("Control Service Success\n");
	}
	else {
		printf("Control Service Failed\n");
		CloseServiceHandle(hOpenService);
		CloseServiceHandle(hOpenSCManager);
	}

	// STEP 5 --> wait for Service to stop
	while (sStatusProcess.dwCurrentState != SERVICE_STOP) {

		// STEP 6 --> Inside while loop query the service
		bQueryServiceStatus = QueryServiceStatusEx(
										hOpenService,
										SC_STATUS_PROCESS_INFO,
										(LPBYTE)&sStatusProcess,
										sizeof(SERVICE_STATUS_PROCESS),
										&dByteNeeded
									);

		if (bQueryServiceStatus == TRUE) {
			printf("QueryServiceStatusEx Failed\n");
			CloseServiceHandle(hOpenService);
			CloseServiceHandle(hOpenSCManager);
            return FALSE;
		}
		else {
			printf("QueryServiceStatusEx Success\n");
		}

		// STEP 7 --> Inside while loop, check the current state of service
		if (sStatusProcess.dwCurrentState == SERVICE_STOPPED) {
			printf("SErvice Stopped Succesfully\n");
			break;
		}
		else {
			printf("Service Stopped failed\n");
			CloseServiceHandle(hOpenService);
			CloseServiceHandle(hOpenSCManager);
            return FALSE;
		}
	}
	CloseServiceHandle(hOpenService);
	CloseServiceHandle(hOpenSCManager);
    return TRUE;
}
