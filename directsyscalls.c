//tool written by b4shcr00k

#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
#include <processthreadsapi.h>
//not actually linking to ntdll this pragma comment is used for InitializeObjectAttributes
#pragma comment(lib, "ntdll.lib")

#define RTL_USER_PROC_PARAMS_NORMALIZED 1
//the actual function we will execute 
typedef NTSTATUS(NTAPI* NtCreateFileFn)(
	PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
	PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG,
	ULONG, ULONG, ULONG, PVOID, ULONG);

//assembly stubs to execute the direct syscall
//sets the syscall id 
EXTERN_C VOID HeavensDecent(__int64 syscallId);
//executes the syscall
EXTERN_C NTSTATUS HeavensGate();


//this function gets the syscall id
__int64 GetNtCreateProcessAddress(char *funcname)
{
	HANDLE hProcess = GetCurrentProcess();
	if (!hProcess)
	{
		printf("[-] Failed To Get A handle To The Current Process\n");
	}
	HANDLE hModule = GetModuleHandleA("ntdll.dll");
	if (!hModule)
	{
		printf("[-] Failed To Get A handle To NtDll\n");
	}
	// we get the address of the function we want to extract its syscall id
	FARPROC FuncAddr = GetProcAddress(hModule, funcname);
	__int64 syscallByte = 0;
	//its usually offset 4 starting from the function address
	if (!ReadProcessMemory(hProcess, (LPCVOID)((PBYTE)FuncAddr + 4), &syscallByte, 1, NULL)) {
		printf("[-] Failed to read syscall byte: %lu\n", GetLastError());
    //lol idk what this is 
		return 898989;
	}
	

	return syscallByte;
}

//actually executes the syscall
void TriggerSyscall(DWORD syscallid)
{
	//things we need for the function
	OBJECT_ATTRIBUTES oa;
	HANDLE fileHandle = NULL;
	NTSTATUS status = NULL;
	UNICODE_STRING fileName;
	IO_STATUS_BLOCK osb;
	RtlInitUnicodeString(&fileName, (PCWSTR)L"\\??\\PUT YOUR PATH HERE USING // AND ENDING WITH THE FILE NAME EX: C://User//Desktop//file.txt");
	ZeroMemory(&osb, sizeof(IO_STATUS_BLOCK));
	InitializeObjectAttributes(&oa, &fileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

	//passes the syscall id to the assembly stub (uses windows calling convention)
	HeavensDecent(syscallid);
	//we cast the assembly stub into the function we wanna execute 
	NtCreateFileFn fn = (NtCreateFileFn)HeavensGate;
	//executes the directsyscall
	status = fn(
		&fileHandle,
		FILE_GENERIC_WRITE,
		&oa,
		&osb,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_WRITE,
		FILE_OVERWRITE_IF,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);


	if (status == 0) {
		printf("[+] Syscall Executed And File Created\n");

	
	}
	else {
		printf("[-] NtCreateFile failed: 0x%X\n", status);
	}
}



int main(int argc, char **argv)
{
	
	if (argc < 2)
	{
		printf("--------Usage : HeavensGate.exe <option> <argument> (-h to show the help message)\n");
		printf("[ERROR]\tYou Must Atleast choose one option\n");
		return 1;
	}
	else 
	{
		if (strcmp(argv[1], "-h") == 0)
		{
			printf("\n-id\textracts the syscall id of the given function EX : -id NtCreateFile\n-t\texecutes NtCreateFile as a proof of concept\n-h\tprints this message\n ");

		}
		else if (strcmp(argv[1],"-id") == 0 )
		{
			if (argv[2] == NULL)
			{
				printf("[ERROR]\tYou didn't give a function name\n");
				return 1;
			}
			else {
				char* funcname = argv[2];
				__int64 syscall = GetNtCreateProcessAddress(funcname);
				if (syscall == 898989)
				{
					return 1;
				}
				printf("SysCall Number is : %X", syscall);
			}
		}
		else if (strcmp(argv[1], "-t") == 0)
		{
			printf("[NOTE]\tMake Sure You Changed The Path\n");
			char* funcname = "NtCreateFile";
			__int64 syscall = GetNtCreateProcessAddress(funcname);
			TriggerSyscall(syscall);
			
		}
	}

	return 0;
}
